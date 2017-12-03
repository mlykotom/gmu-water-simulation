#ifndef CCOLLISIONGEOMETRY_H
#define CCOLLISIONGEOMETRY_H

#include <Qt3DCore/QEntity>
#include <QVector3D>
#include <QGeometry>

struct sVertex
{
    //position
    QVector3D x, y, z;

    //normal
    QVector3D normal;

};

struct sFace
{
    //vertices
    sVertex v0, v1, v2;

    //normal
    QVector3D normal;
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