#ifndef CCOLLISIONGEOMETRY_H
#define CCOLLISIONGEOMETRY_H

#include <Qt3DCore/QEntity>
#include <QVector3D>
#include <QGeometry>

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
        m_normal = QVector3D::crossProduct(v0.m_pos,v1.m_pos);
    }

    sFace(sVertex v0, sVertex v1, sVertex v2, QVector3D normal)
        :m_v0(v0),
        m_v1(v1),
        m_v2(v2),
        m_normal(normal)
    {}
};

class CCollisionGeometry : public Qt3DCore::QEntity
{
    Q_OBJECT

public:
    CCollisionGeometry(Qt3DRender::QGeometry *geometry, Qt3DCore::QNode *parent = 0);
    ~CCollisionGeometry();

private: //methods
    void init();

private: //attributes
    Qt3DRender::QGeometry *m_geometry;
    QVector<sVertex> m_vertices;
    QVector<sFace> m_faces;

};

#endif