#ifndef CCOLLISIONGEOMETRY_H
#define CCOLLISIONGEOMETRY_H

//qt
#include <Qt3DCore/QEntity>
#include <QVector3D>
#include <QVector>
#include <QGeometry>
#include <QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QBufferDataGeneratorPtr>
#include <Qt3DRender/QBufferDataGenerator>

//std
#include <limits>

//local
#include <CParticle.h>

#define WALL_K 10000.0 // wall spring constant
#define WALL_DAMPING (-0.9) // wall damping constant

struct sWall
{
    cl_float3 normal;
    cl_float3 position;

    sWall() = default;
    sWall(const cl_float3 &norm, const cl_float3 &pos) : normal(norm), position(pos) {}
};

struct sVertex
{
    //position
    QVector3D m_pos;

    //normal
    QVector3D m_normal;

    sVertex() = default;
    explicit sVertex(QVector3D pos) : m_pos(pos) {}
    sVertex(QVector3D pos, QVector3D normal) : m_pos(pos), m_normal(normal) {}
};

struct sFace
{
    //vertices
    sVertex m_v0, m_v1, m_v2;
    QVector<sVertex> m_vertices;
    //normal
    QVector3D m_normal;

    sFace() = default;

    sFace(sVertex v0, sVertex v1, sVertex v2)
        : m_v0(v0),
          m_v1(v1),
          m_v2(v2)
    {
        //m_normal = QVector3D::crossProduct(v0.m_pos,v1.m_pos);
        m_normal = v0.m_normal;
        m_vertices.push_back(m_v0);
        m_vertices.push_back(m_v1);
        m_vertices.push_back(m_v2);
    }

    sFace(sVertex v0, sVertex v1, sVertex v2, QVector3D normal)
        : m_v0(v0),
          m_v1(v1),
          m_v2(v2),
          m_normal(normal)
    {
        m_vertices.push_back(m_v0);
        m_vertices.push_back(m_v1);
        m_vertices.push_back(m_v2);
    }
};

struct sBoundingBox
{
    QVector3D m_min;
    QVector3D m_max;

    QVector<sWall> m_walls;

    sBoundingBox()
    {
        m_min = QVector3D(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        m_max = QVector3D(-1 * std::numeric_limits<float>::max(), -1 * std::numeric_limits<float>::max(), -1 * std::numeric_limits<float>::max());
        m_walls.resize(6);
    }

    void expandBy(QVector3D vec)
    {
        minimize(vec);
        maximize(vec);
    }

    void minimize(QVector3D vec)
    {
        m_min.setX(std::min(m_min.x(), vec.x()));
        m_min.setY(std::min(m_min.y(), vec.y()));
        m_min.setZ(std::min(m_min.z(), vec.z()));

        m_walls[0] = sWall({-1, 0, 0}, {m_min.x(), 0, 0}); //left
        m_walls[1] = sWall({0, -1, 0}, {0, m_min.y(), 0}); //bottom
        m_walls[2] = sWall({0, 0, -1}, {0, 0, m_min.z()}); //back
    }

    void maximize(QVector3D vec)
    {
        m_max.setX(std::max(m_max.x(), vec.x()));
        m_max.setY(std::max(m_max.y(), vec.y()));
        m_max.setZ(std::max(m_max.z(), vec.z()));

        m_walls[3] = sWall({1, 0, 0}, {m_max.x(), 0, 0}); //right
        m_walls[4] = sWall({0, 1, 0}, {0, m_max.y(), 0}); //top
        m_walls[5] = sWall({0, 0, 1}, {0, 0, m_max.z()}); //front
    }
};

class CCollisionGeometry: public Qt3DCore::QEntity
{
Q_OBJECT

public:
    explicit CCollisionGeometry(Qt3DRender::QGeometry *geometry, Qt3DCore::QNode *parent = 0);
    ~CCollisionGeometry() override;

    QVector3D inverseBounce(QVector3D pos, QVector3D velocity);
    QVector3D inverseBoundingBoxBounce(QVector3D &pos, QVector3D &velocity);

    const sBoundingBox &getBoundingBox() { return m_boundingBox; }
private: //methods
    void init();

    template<typename T>
    void extractVertices(Qt3DRender::QAttribute *posAttribute, Qt3DRender::QAttribute *normalAttribute);

    template<typename T>
    void extractFaces(Qt3DRender::QAttribute *indexAttribute);

private: //attributes
    Qt3DRender::QGeometry *m_geometry;
    QVector<sVertex> m_vertices;
    QVector<sFace> m_faces;

    sBoundingBox m_boundingBox;
};

template<typename T>
inline void CCollisionGeometry::extractVertices(Qt3DRender::QAttribute *posAttribute, Qt3DRender::QAttribute *normalAttribute)
{
    m_vertices.clear();

    int vertexSize = posAttribute->vertexSize();
    int vertexCount = posAttribute->count();
    int vertexByteOffset = posAttribute->byteOffset() / sizeof(T);
    int vertexByteStride = posAttribute->byteStride() / sizeof(T);

    int normalByteStride = posAttribute->byteStride() / sizeof(T);
    int normalByteOffset = normalAttribute->byteOffset() / sizeof(T);
    int normalCount = normalAttribute->count();

    Qt3DRender::QBuffer *posBuffer = posAttribute->buffer();
    Qt3DRender::QBufferDataGeneratorPtr generator = posBuffer->dataGenerator();
    Qt3DRender::QBufferDataGenerator * gen = generator.operator->();
    QByteArray posArr = gen->operator()();
    T *vertices = reinterpret_cast<T *>(posArr.data());

    int size = posArr.size() / sizeof(T);

    for (int i = 0; i < size; i += vertexByteStride) {

        QVector3D pos(vertices[i + vertexByteOffset], vertices[i + vertexByteOffset + 1], vertices[i + vertexByteOffset + 2]);
        QVector3D normal(vertices[i + normalByteOffset], vertices[i + normalByteOffset + 1], vertices[i + normalByteOffset + 2]);

        m_vertices.push_back(sVertex(pos, normal));
    }
}

template<typename T>
inline void CCollisionGeometry::extractFaces(Qt3DRender::QAttribute *indexAttribute)
{
    Qt3DRender::QBuffer *indexBuffer = indexAttribute->buffer();
    Qt3DRender::QBufferDataGeneratorPtr indexGenerator = indexBuffer->dataGenerator();
    Qt3DRender::QBufferDataGenerator * indexGen = indexGenerator.operator->();
    QByteArray indexArr = indexGen->operator()();
    T *indices = reinterpret_cast<T *>(indexArr.data());

    int size = indexArr.size() / sizeof(T);

    for (int i = 0; i < size; i += 3) {
        m_faces.push_back(sFace(m_vertices.at(indices[i]), m_vertices.at(indices[i + 1]), m_vertices.at(indices[i + 2])));
    }
}

#endif
