#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/config.h>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <limits>
#include "rhimodel.h"
#include "rhimesh.h"

namespace photon {

namespace {

QMatrix4x4 convert(const aiMatrix4x4 &t)
{
    return QMatrix4x4(t.a1, t.a2, t.a3, t.a4,
                      t.b1, t.b2, t.b3, t.b4,
                      t.c1, t.c2, t.c3, t.c4,
                      t.d1, t.d2, t.d3, t.d4);
}

int axisFromSuffix(const QString &name)
{
    if (name.endsWith("_x", Qt::CaseInsensitive)) return 0;
    if (name.endsWith("_y", Qt::CaseInsensitive)) return 1;
    if (name.endsWith("_z", Qt::CaseInsensitive)) return 2;
    return 1; // default to Y
}

// Builds an RhiMesh (vec3 pos + vec3 normal) from one aiMesh and returns its local AABB.
RhiMesh *meshFromAi(const aiMesh *mesh, QVector3D &outMin, QVector3D &outMax)
{
    const int n = int(mesh->mNumVertices);
    QByteArray verts;
    verts.reserve(n * 6 * int(sizeof(float)));
    outMin = QVector3D( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    outMax = QVector3D(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

    for (int i = 0; i < n; ++i) {
        const aiVector3D &p = mesh->mVertices[i];
        const aiVector3D nrm = mesh->mNormals ? mesh->mNormals[i] : aiVector3D(0, 1, 0);
        const float f[6] = { p.x, p.y, p.z, nrm.x, nrm.y, nrm.z };
        verts.append(reinterpret_cast<const char *>(f), sizeof(f));
        outMin.setX(qMin(outMin.x(), p.x)); outMin.setY(qMin(outMin.y(), p.y)); outMin.setZ(qMin(outMin.z(), p.z));
        outMax.setX(qMax(outMax.x(), p.x)); outMax.setY(qMax(outMax.y(), p.y)); outMax.setZ(qMax(outMax.z(), p.z));
    }

    QByteArray indices;
    for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
        const aiFace &face = mesh->mFaces[f];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            const quint16 idx = quint16(face.mIndices[j]);
            indices.append(reinterpret_cast<const char *>(&idx), sizeof(idx));
        }
    }

    auto *out = new RhiMesh;
    out->setVertexData(verts, n);
    out->setIndexData(indices, int(indices.size() / sizeof(quint16)));
    return out;
}

QColor materialColor(const aiScene *scene, const aiMesh *mesh)
{
    if (mesh->mMaterialIndex >= scene->mNumMaterials)
        return QColor(190, 190, 200);
    aiColor4D c;
    if (aiGetMaterialColor(scene->mMaterials[mesh->mMaterialIndex], AI_MATKEY_COLOR_DIFFUSE, &c) == AI_SUCCESS)
        return QColor::fromRgbF(qBound(0.0, double(c.r), 1.0), qBound(0.0, double(c.g), 1.0), qBound(0.0, double(c.b), 1.0));
    return QColor(190, 190, 200);
}

} // namespace

static void traceNode(const RhiModel::Node &n, int depth)
{
    QString roles;
    if (n.panAxis  >= 0) roles += QString(" PAN[%1]").arg("xyz"[n.panAxis]);
    if (n.tiltAxis >= 0) roles += QString(" TILT[%1]").arg("xyz"[n.tiltAxis]);
    if (n.emitter)       roles += " LAMP";
    if (n.lens)          roles += " LENS";
    qWarning().noquote() << QString(depth * 2, ' ') + "|-" << (n.name.isEmpty() ? "<unnamed>" : n.name)
                         << QString("(meshes %1)").arg(n.meshes.size()) << roles;
    for (const RhiModel::Node &c : n.children)
        traceNode(c, depth + 1);
}

static void deleteNodeMeshes(RhiModel::Node &n)
{
    for (auto *m : n.meshes)
        delete m;            // RhiMesh dtor releases its GPU buffers
    n.meshes.clear();
    for (auto &c : n.children)
        deleteNodeMeshes(c);
}

RhiModel::~RhiModel()
{
    deleteNodeMeshes(m_root);
}

static void processNode(const aiNode *node, const aiScene *scene, const QMatrix4x4 &parentAccum,
                        RhiModel::Node &out, bool &hasEmitter, QVector3D &gMin, QVector3D &gMax)
{
    out.name = node->mName.C_Str();
    out.local = convert(node->mTransformation);
    const QMatrix4x4 accum = parentAccum * out.local;

    // Some exporters (e.g. glTF) prefix node names with underscores — strip leading
    // ones so the role prefixes still match (e.g. "_pan_y" -> "pan_y").
    QString lname = out.name.toLower();
    while (lname.startsWith('_'))
        lname.remove(0, 1);
    if (lname.startsWith("pan_"))
        out.panAxis = axisFromSuffix(lname);
    if (lname.startsWith("tilt_"))
        out.tiltAxis = axisFromSuffix(lname);
    if (lname == "lamp") {
        out.emitter = true;
        hasEmitter = true;
    }
    if (lname.startsWith("lens"))
        out.lens = true;

    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        QVector3D mn, mx;
        out.meshes.append(meshFromAi(mesh, mn, mx));
        if (i == 0)
            out.color = materialColor(scene, mesh);
        // Expand global bounds by this mesh's AABB transformed into model space.
        for (int cx = 0; cx < 2; ++cx)
            for (int cy = 0; cy < 2; ++cy)
                for (int cz = 0; cz < 2; ++cz) {
                    const QVector3D corner = accum.map(QVector3D(cx ? mx.x() : mn.x(),
                                                                cy ? mx.y() : mn.y(),
                                                                cz ? mx.z() : mn.z()));
                    gMin.setX(qMin(gMin.x(), corner.x())); gMin.setY(qMin(gMin.y(), corner.y())); gMin.setZ(qMin(gMin.z(), corner.z()));
                    gMax.setX(qMax(gMax.x(), corner.x())); gMax.setY(qMax(gMax.y(), corner.y())); gMax.setZ(qMax(gMax.z(), corner.z()));
                }
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        RhiModel::Node child;
        processNode(node->mChildren[i], scene, accum, child, hasEmitter, gMin, gMax);
        out.children.append(child);
    }
}

RhiModel *RhiModel::load(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "RhiModel: cannot open" << path;
        return nullptr;
    }
    const QByteArray data = file.readAll();

    Assimp::Importer importer;
    // Keep meshes under the 16-bit index limit so we can use uint16 index buffers.
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, 60000);
    // The format hint (file extension) lets assimp pick the right reader when reading
    // from memory — without it binary containers like .glb can't be sniffed.
    const QByteArray hint = QFileInfo(path).suffix().toLower().toUtf8();
    const aiScene *scene = importer.ReadFileFromMemory(
        data.constData(), data.size(),
        aiProcess_Triangulate | aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices | aiProcess_SplitLargeMeshes,
        hint.constData());

    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
        qWarning() << "RhiModel: assimp failed for" << path << ":" << importer.GetErrorString();
        return nullptr;
    }

    auto *model = new RhiModel;
    QVector3D gMin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    QVector3D gMax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    processNode(scene->mRootNode, scene, QMatrix4x4(), model->m_root, model->m_hasEmitter, gMin, gMax);

    if (gMin.x() <= gMax.x()) {   // had geometry
        model->m_min = gMin;
        model->m_max = gMax;
    }
    qWarning() << "RhiModel: loaded" << path << "emitter" << model->m_hasEmitter
               << "bounds" << model->m_min << model->m_max;
    qWarning().noquote() << "RhiModel: node tree for" << path;
    traceNode(model->m_root, 0);
    return model;
}

void RhiModel::uploadNode(Node &n, QRhi *rhi, QRhiResourceUpdateBatch *batch)
{
    for (auto *m : n.meshes)
        m->upload(rhi, batch);
    for (auto &c : n.children)
        uploadNode(c, rhi, batch);
}

void RhiModel::upload(QRhi *rhi, QRhiResourceUpdateBatch *batch)
{
    uploadNode(m_root, rhi, batch);
}

void RhiModel::releaseNode(Node &n)
{
    for (auto *m : n.meshes)
        m->release();
    for (auto &c : n.children)
        releaseNode(c);
}

void RhiModel::releaseGpu()
{
    releaseNode(m_root);
}

} // namespace photon
