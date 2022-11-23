#include "trussgeometry.h"
#include "Pipe.h"
#include "Line.h"

namespace photon {

class TrussGeometry::Impl
{
public:
    Impl(TrussGeometry *);
    void updateVertices();
    void addTube(const std::vector<Vector3> &t_path, float t_radius, std::vector<Vector3> &t_points, std::vector<quint16> &t_faces, uint t_arcSegments = 12);

    float segmentLength = 5;
    float length = 40;
    float angle = 0.0f;
    int beams = 4;
    float offset = 6.0f;
    float radius = 2.0f;

private:
    QAttribute *m_positionAttribute;
    QAttribute *m_normalAttribute;
    QAttribute *m_indexAttribute;
    QVector<QVector3D> m_points;
    Qt3DCore::QBuffer *m_vertexBuffer;
    Qt3DCore::QBuffer *m_indexBuffer;
    TrussGeometry *m_facade;
};

std::vector<Vector3> buildCircle(float radius, int steps)
{
    std::vector<Vector3> points;
    if(steps < 2) return points;

    const float PI2 = acos(-1) * 2.0f;
    float x, y, a;
    for(int i = 0; i <= steps; ++i)
    {
        a = PI2 / steps * i;
        x = radius * cosf(a);
        y = radius * sinf(a);
        points.push_back(Vector3(x, y, 0));
    }
    return points;
}


TrussGeometry::Impl::Impl(TrussGeometry *t_facade):m_facade(t_facade)
{
    m_positionAttribute = new QAttribute(t_facade);
    m_normalAttribute = new QAttribute(t_facade);
    m_indexAttribute = new QAttribute(t_facade);
    m_vertexBuffer = new Qt3DCore::QBuffer(t_facade);
    m_indexBuffer = new Qt3DCore::QBuffer(t_facade);

    //m_points.append(QVector3D(-10,-10,0));
    m_points.append(QVector3D(30,-10,0));
    //m_points.append(QVector3D(8,10,0));
    m_points.append(QVector3D(-10,10,0));


    const quint32 elementSize = 3 + 3;
    const quint32 stride = elementSize * sizeof(float);

    m_positionAttribute->setName(QAttribute::defaultPositionAttributeName());
    m_positionAttribute->setVertexBaseType(QAttribute::Float);
    m_positionAttribute->setVertexSize(3);
    m_positionAttribute->setAttributeType(QAttribute::VertexAttribute);
    m_positionAttribute->setBuffer(m_vertexBuffer);
    m_positionAttribute->setByteStride(stride);
    //m_positionAttribute->setCount(nVerts);

    m_normalAttribute->setName(QAttribute::defaultNormalAttributeName());
    m_normalAttribute->setVertexBaseType(QAttribute::Float);
    m_normalAttribute->setVertexSize(3);
    m_normalAttribute->setAttributeType(QAttribute::VertexAttribute);
    m_normalAttribute->setBuffer(m_vertexBuffer);
    m_normalAttribute->setByteStride(stride);
    m_normalAttribute->setByteOffset(3 * sizeof(float));
    //m_normalAttribute->setCount(nVerts);

    m_indexAttribute->setAttributeType(QAttribute::IndexAttribute);
    m_indexAttribute->setVertexBaseType(QAttribute::UnsignedShort);
    m_indexAttribute->setBuffer(m_indexBuffer);

    //m_indexAttribute->setCount(faces * 3);


    m_facade->addAttribute(m_positionAttribute);
    m_facade->addAttribute(m_normalAttribute);
    m_facade->addAttribute(m_indexAttribute);

    updateVertices();
}


void TrussGeometry::Impl::addTube(const std::vector<Vector3> &t_path, float t_radius, std::vector<Vector3> &t_points, std::vector<quint16> &t_faces, uint t_arcSegments)
{
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;

    Pipe pipe;
    auto circlePoints = buildCircle(t_radius, t_arcSegments);
    pipe.set(t_path, circlePoints);

    int initialPointCount = t_points.size() / 2.0;
    int count = pipe.getContourCount();
    for(int i = 0; i < count; ++i)
    {
        std::vector<Vector3> contour = pipe.getContour(i);
        std::vector<Vector3> normal = pipe.getNormal(i);

        for(int j = 0; j < (int)contour.size(); ++j)
        {
            t_points.push_back(contour[j]);
            t_points.push_back(normal[j]);
        }
    }

    for (int ring = 0; ring < t_path.size() - 1; ++ring) {
        const int ringIndexStart = ring * (t_arcSegments + 1);
        const int nextRingIndexStart = (ring + 1) * (t_arcSegments + 1);

        for (int slice = 0; slice < t_arcSegments; ++slice) {
            const int nextSlice = slice + 1;

            t_faces.push_back((ringIndexStart + nextSlice) + initialPointCount);
            t_faces.push_back((nextRingIndexStart + slice) + initialPointCount);
            t_faces.push_back((ringIndexStart + slice) + initialPointCount);

            t_faces.push_back((nextRingIndexStart + nextSlice) + initialPointCount);
            t_faces.push_back((nextRingIndexStart + slice) + initialPointCount);
            t_faces.push_back((ringIndexStart + nextSlice) + initialPointCount);
        }
    }
}

std::vector<Vector3> subdivide(const std::vector<Vector3> &t_points, float t_maxDistance)
{
    if(t_points.size() < 2)
        return t_points;

    std::vector<Vector3> output;
    auto it = t_points.cbegin();
    output.push_back(*it);
    Vector3 lastPosition = *it;

    for(++it; it != t_points.cend(); ++it)
    {
        Vector3 currentPt = *it;
        float distance = lastPosition.distance(currentPt);
        int subdivisions = ceil(distance/t_maxDistance);
        float subIncrement = (distance/subdivisions)/distance;
        for(int i = 1; i < subdivisions; ++i)
        {
            output.push_back(Line::pointOnLine(lastPosition, currentPt, subIncrement * i));
        }

        output.push_back(currentPt);
        lastPosition = currentPt;
    }

    return output;
}

void TrussGeometry::Impl::updateVertices()
{
    const int sides = beams;
    Pipe offsetPipe;
    std::vector<Vector3> points;

    points.push_back(Vector3(-length*.5f,0,0));
    points.push_back(Vector3(length*.5f,0,0));

    points = subdivide(points, segmentLength);

    auto circlePoints = buildCircle(offset, sides);
    circlePoints.pop_back();

    offsetPipe.set(points, circlePoints);

    std::vector<std::vector<Vector3>> contours;
    contours.resize(sides);
    for(int i = 0; i < offsetPipe.getContourCount(); ++i)
    {
        //contours.push_back(offsetPipe.getContour(i));
        int contourCounter = 0;

        std::vector<Vector3> contour = offsetPipe.getContour(i);
        for(int j = 0; j < (int)contour.size(); ++j)
        {
            contours[contourCounter].push_back(contour[j]);
            ++contourCounter;
        }

    }

    std::vector<Vector3> pointsOut;
    std::vector<quint16> indicesOut;

    for(auto it = contours.cbegin(); it != contours.cend(); ++it)
    {
        addTube(*it, radius, pointsOut, indicesOut);
    }

    for(int i = 0; i < contours.size(); ++i)
    {
        auto currentContour = contours[i];
        auto nextContour = contours[(i+1)%contours.size()];


        for(int k = 0; k < currentContour.size() - 2; k += 2)
        {

            std::vector<Vector3> bracePoints;
            bracePoints.push_back(currentContour[k]);
            bracePoints.push_back(nextContour[k+1]);
            addTube(bracePoints, radius*.5f, pointsOut, indicesOut,8);


            std::vector<Vector3> bracePoints2;
            bracePoints2.push_back(nextContour[k+1]);
            bracePoints2.push_back(currentContour[k+2]);
            addTube(bracePoints2, radius*.5f, pointsOut, indicesOut,8);
        }

    }

    QByteArray verticesData;
    verticesData.resize(sizeof(Vector3) * pointsOut.size());
    float *verticesPtr = reinterpret_cast<float*>(verticesData.data());
    for(Vector3 v : pointsOut)
    {
        *verticesPtr++ = v.x;
        *verticesPtr++ = v.y;
        *verticesPtr++ = v.z;
    }

    const int indexSize = sizeof(quint16);
    QByteArray indicesBytes;
    indicesBytes.resize(indicesOut.size() * indexSize);
    quint16 *indicesPtr = reinterpret_cast<quint16*>(indicesBytes.data());
    for(quint16 v : indicesOut)
        *indicesPtr++ = v;

    m_positionAttribute->setCount(static_cast<uint>(pointsOut.size()/2));
    m_normalAttribute->setCount(static_cast<uint>(pointsOut.size()/2));

    m_vertexBuffer->setData(verticesData);

    m_indexAttribute->setCount(static_cast<uint>(indicesOut.size()));
    m_indexBuffer->setData(indicesBytes);
}

TrussGeometry::TrussGeometry(QNode *parent)  : QGeometry(parent), m_impl(new Impl(this))
{

}

TrussGeometry::~TrussGeometry()
{
    delete m_impl;
}

void TrussGeometry::setSegmentLength(float t_value)
{
    m_impl->segmentLength = t_value;
    m_impl->updateVertices();
}

void TrussGeometry::setRadius(float t_value)
{
    m_impl->radius = t_value;
    m_impl->updateVertices();
}

void TrussGeometry::setBeams(uint t_beams)
{
    m_impl->beams = t_beams;
    m_impl->updateVertices();
}

void TrussGeometry::setOffset(float t_offset)
{
    m_impl->offset = t_offset;
    m_impl->updateVertices();
}

void TrussGeometry::setLength(float t_length)
{
    m_impl->length = t_length;
    m_impl->updateVertices();
}

void TrussGeometry::setAngle(float t_angle)
{
    m_impl->angle = t_angle;
    m_impl->updateVertices();
}

float TrussGeometry::radius() const
{
    return m_impl->radius;
}

float TrussGeometry::segmentLength() const
{
    return m_impl->segmentLength;
}

float TrussGeometry::offset() const
{
    return m_impl->offset;
}

float TrussGeometry::length() const
{
    return m_impl->length;
}

float TrussGeometry::angle() const
{
    return m_impl->angle;
}

uint TrussGeometry::beams() const
{
    return m_impl->beams;
}

} // namespace photon
