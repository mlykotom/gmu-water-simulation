#ifndef CGRID_H
#define CGRID_H

//Qt3D
#include <QGeometry>
#include <Qt3DRender/QMaterial>
#include <Qt3DExtras/QCuboidGeometry>
#include <QVector3D>

//local includes
#include "CParticle.h"
#include "renderableentity.h"
#include "CCollisionGeometry.h"

class CGrid : RenderableEntity
{

public: //methods
    explicit CGrid(QVector3D size, QVector3D resolution, Qt3DCore::QNode *parent = 0);

    CGrid(Qt3DCore::QNode *parent = 0);
    ~CGrid();


    std::vector<CParticle *> *getData() const { return m_data; }
    std::vector<CParticle *> &at(int x, int y, int z)
    {
        return m_data[x + y * m_ResY + z * m_ResZ * m_ResY];
    }

    const int xRes() const { return m_ResX; }
    const int yRes() const { return m_ResY; }
    const int zRes() const { return m_ResZ; }

    const int &getCellCount() const { return m_cell_count; }

    std::vector<CParticle *> getNeighborsCells(int x, int y, int z);

    CCollisionGeometry *getCollisionGeometry() { return m_collisionGeometry; }
    //QVector3D inverseBounce();

private: //attributes
    Qt3DExtras::QCuboidGeometry *m_geometry;
    Qt3DRender::QMaterial *m_material;
    Qt3DRender::QGeometryRenderer *m_meshRenderer;
    CCollisionGeometry *m_collisionGeometry;

    std::vector<CParticle *> *m_data;
    int m_cell_count, m_ResX, m_ResY, m_ResZ;
    QVector3D m_resolution;
};

#endif //WATERSURFACESIMULATION_CGRID_H
