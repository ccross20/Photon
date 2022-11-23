#include "modelloader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <QFile>
#include <QDebug>
#include "component/abstractmesh.h"
#include "component/transformcomponent.h"
#include "component/materialcomponent.h"
#include "entity.h"

namespace photon {

ModelLoader::ModelLoader()
{

}


AbstractMesh *processMesh(aiMesh *mesh, const aiScene *scene)
{
    AbstractMesh *outputMesh = new AbstractMesh;
    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QVector<GLushort> indices;
    //vector<Texture> textures;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        if(mesh->mVertices)
        {
            QVector3D vertex;
            vertex.setX(mesh->mVertices[i].x);
            vertex.setY(mesh->mVertices[i].y);
            vertex.setZ(mesh->mVertices[i].z);
            vertices.push_back(vertex);
        }


        if(mesh->mNormals)
        {
            QVector3D normal;
            normal.setX(mesh->mNormals[i].x);
            normal.setY(mesh->mNormals[i].y);
            normal.setZ(mesh->mNormals[i].z);
            normals.push_back(normal);
        }

    }

    outputMesh->setVertices(vertices);
    outputMesh->setNormals(normals);

    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    outputMesh->setIndices(indices);

    // process material
    if(mesh->mMaterialIndex >= 0)
    {

    }

    return outputMesh;
}

Entity *processNode(aiNode *node, const aiScene *scene)
{
    // process all the node's meshes (if any)

    qDebug() << "Name: " << node->mName.C_Str();

    QString nodeName = node->mName.C_Str();

    Entity *entity = new Entity;
    entity->setName(nodeName);

    auto t = node->mTransformation;

    QMatrix4x4 mat(t.a1, t.a2, t.a3, t.a4,
                   t.b1, t.b2, t.b3, t.b4,
                   t.c1, t.c2, t.c3, t.c4,
                   t.d1, t.d2, t.d3, t.d4);

    TransformComponent *xform = new TransformComponent;
    xform->setLocalMatrix(mat);

    entity->addComponent(xform);

    /*
    if(node->mMetaData)
    {
        for(uint i = 0; i < node->mMetaData->mNumProperties; ++i)
        {
            qDebug() << node->mMetaData->mKeys[i].C_Str();
        }
    }
    */


    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        entity->addComponent(processMesh(mesh, scene));
    }
    // then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        Entity *childEntity = processNode(node->mChildren[i], scene);
        childEntity->setParent(entity);

        auto childXForm = childEntity->findComponent<TransformComponent*>();
        if(childXForm)
            childXForm->setParent(xform);

    }

    if(entity->findComponent<AbstractMesh*>())
    {
        MaterialComponent *material = new MaterialComponent;
        material->setFragmentSource(":/resources/shader/basic_lighting.frag");
        material->setVertexSource(":/resources/shader/basic_lighting.vert");
        entity->addComponent(material);
    }

    return entity;
}

Entity *ModelLoader::loadResource(QString path)
{
    QFile file(path);

    file.open(QIODevice::ReadOnly);

    QByteArray fileContents = file.readAll();

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFileFromMemory(fileContents.data(), fileContents.length(), aiProcess_Triangulate | aiProcess_FlipUVs);

    qDebug() << "File length:" << fileContents.length();

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        qDebug() << "ERROR::ASSIMP::" << importer.GetErrorString();
        return nullptr;
    }

    Entity *modelRoot = processNode(scene->mRootNode, scene);

    return modelRoot;
}

} // namespace photon
