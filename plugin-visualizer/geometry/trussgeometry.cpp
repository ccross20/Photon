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

    float segmentLength = .25;
    float length = 4;
    float angle = 9.0f;
    int beams = 4;
    float offset = .25f;
    float radius = .025f;

private:
    QVector<Vector3> m_points;
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


    //m_points.append(Vector3(-10,-10,0));
    m_points.append(Vector3(0,-10,0));
    //m_points.append(Vector3(8,10,0));
    m_points.append(Vector3(0,10,0));


    updateVertices();
}

QVector3D convertPt(Vector3 pt)
{
    return QVector3D{pt.x, pt.y, pt.z};
}


void TrussGeometry::Impl::addTube(const std::vector<Vector3> &t_path, float t_radius, std::vector<Vector3> &t_points, std::vector<quint16> &t_faces, uint t_arcSegments)
{
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;

    Pipe pipe;
    auto circlePoints = buildCircle(t_radius, t_arcSegments);
    pipe.set(t_path, circlePoints);

    int initialPointCount = t_points.size();
    int count = pipe.getContourCount();
    for(int i = 0; i < count; ++i)
    {
        std::vector<Vector3> contour = pipe.getContour(i);
        std::vector<Vector3> normal = pipe.getNormal(i);

        for(int j = 0; j < (int)contour.size(); ++j)
        {
            t_points.push_back(contour[j]);
            //t_points.push_back(normal[j]);
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


    //addTube(points, radius, pointsOut, indicesOut);


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




    QVector<QVector3D> pointList;
    QVector<QVector3D> normalList;
    QVector<GLushort> indicesList;


    for(Vector3 v : pointsOut)
    {
        pointList.append(convertPt(v));
    }
    for(quint16 v : indicesOut)
        indicesList.append(v);

    //qDebug() << "Total" << pointList.size() << indicesList.size();
    normalList.resize(pointList.length());

    for(int i = 0; i < indicesList.length(); i += 3)
    {
        int ai = indicesList[i];
        int bi = indicesList[i+1];
        int ci = indicesList[i+2];
        QVector3D a = pointList[ai];
        QVector3D b = pointList[bi];
        QVector3D c = pointList[ci];
        auto dir = QVector3D::crossProduct(b - a, c - a);
        auto norm = dir.normalized();
        normalList[ai] = norm;
        normalList[bi] = norm;
        normalList[ci] = norm;
    }

    m_facade->setIndices(indicesList);
    m_facade->setVertices(pointList);
    m_facade->setNormals(normalList);


}

TrussGeometry::TrussGeometry()  : AbstractMesh(), m_impl(new Impl(this))
{
    //setPrimitveType(GL_TRIANGLE_STRIP);
}

TrussGeometry::~TrussGeometry()
{
    delete m_impl;
}

void TrussGeometry::setSegmentLength(float t_value)
{
    m_impl->segmentLength = t_value;
    setDirty(Dirty_Rebuild);
}

void TrussGeometry::setRadius(float t_value)
{
    m_impl->radius = t_value;
    setDirty(Dirty_Rebuild);
}

void TrussGeometry::setBeams(uint t_beams)
{
    m_impl->beams = t_beams;
    setDirty(Dirty_Rebuild);
}

void TrussGeometry::setOffset(float t_offset)
{
    m_impl->offset = t_offset;
    setDirty(Dirty_Rebuild);
}

void TrussGeometry::setLength(float t_length)
{
    m_impl->length = t_length;
    setDirty(Dirty_Rebuild);
}

void TrussGeometry::setAngle(float t_angle)
{
    m_impl->angle = t_angle;
    setDirty(Dirty_Rebuild);
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

void TrussGeometry::rebuild(QOpenGLContext *context)
{
    m_impl->updateVertices();
    AbstractMesh::rebuild(context);
}

} // namespace photon
