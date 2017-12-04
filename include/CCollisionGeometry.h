#ifndef CCOLLISIONGEOMETRY_H
#define CCOLLISIONGEOMETRY_H

#include <Qt3DCore/QEntity>
#include <QVector3D>
#include <QVector>
#include <QGeometry>

#include <limits>

struct sVertex
{

    //position
    QVector3D m_pos;

    //normal
    QVector3D m_normal;

    sVertex::sVertex(QVector3D pos)
        :m_pos(pos)
    {
    }

    sVertex(QVector3D pos, QVector3D normal)
        : m_pos(pos),
        m_normal(normal)
    {
    }

    sVertex()
    {
    }
};

struct sFace
{
    //vertices
    sVertex m_v0, m_v1, m_v2;
    QVector<sVertex> m_vertices;
    //normal
    QVector3D m_normal;


    sFace()
    {
    }

    sFace(sVertex v0, sVertex v1, sVertex v2)
        :m_v0(v0),
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
        :m_v0(v0),
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
    //normal, position
    typedef QPair<QVector3D, QVector3D> tWall;
 
    QVector3D m_min;
    QVector3D m_max;

    QVector< tWall > m_walls;

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

        m_walls[0] = tWall(QVector3D(-1, 0, 0), QVector3D(m_min.x(), 0, 0)); //left
        m_walls[1] = tWall(QVector3D(0, -1, 0), QVector3D(0, m_min.y(), 0)); //bottom
        m_walls[2] = tWall(QVector3D(0, 0, -1), QVector3D(0, 0, m_min.z())); //back        
    }

    void maximize(QVector3D vec)
    {
        m_max.setX(std::max(m_max.x(), vec.x()));
        m_max.setY(std::max(m_max.y(), vec.y()));
        m_max.setZ(std::max(m_max.z(), vec.z()));

        m_walls[3] = tWall(QVector3D(1, 0, 0), QVector3D(m_max.x(), 0, 0)); //right
        m_walls[4] = tWall(QVector3D(0, 1, 0), QVector3D(0, m_max.y(), 0)); //top
        m_walls[5] = tWall(QVector3D(0, 0, 1), QVector3D(0, 0, m_max.z())); //front
    }
};

class CCollisionGeometry : public Qt3DCore::QEntity
{
    Q_OBJECT

public:
    CCollisionGeometry(Qt3DRender::QGeometry *geometry, Qt3DCore::QNode *parent = 0);
    ~CCollisionGeometry();

    QVector3D inverseBounce(QVector3D pos, QVector3D velocity);
    QVector3D inverseBoundingBoxBounce(QVector3D pos, QVector3D velocity);

private: //methods
    void init();

private: //attributes
    Qt3DRender::QGeometry *m_geometry;
    QVector<sVertex> m_vertices;
    QVector<sFace> m_faces;

    sBoundingBox m_boundingBox;
};

#endif