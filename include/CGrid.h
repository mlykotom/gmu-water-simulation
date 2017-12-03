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

class CGrid : RenderableEntity
{

public: //methods
    explicit CGrid(int x, int y, int z, Qt3DCore::QNode *parent = 0);
    
    CGrid(Qt3DCore::QNode *parent = 0);
    ~CGrid();


    std::vector<CParticle *> *getData() const { return m_data; }
    std::vector<CParticle *> &at(int x, int y, int z)
    {
        return m_data[x + y * m_y + z * m_z * m_y];
    }

    const int xRes() const { return m_x; }
    const int yRes() const { return m_y; }
    const int zRes() const { return m_z; }
    const int &getCellCount() const { return m_cell_count; }

private: //attributes
    Qt3DExtras::QCuboidGeometry *m_geometry;
    Qt3DRender::QMaterial *m_material;
    Qt3DRender::QGeometryRenderer *m_meshRenderer;
    
    std::vector<CParticle *> *m_data;
    const int m_cell_count, m_x, m_y, m_z;
   
};

#endif //WATERSURFACESIMULATION_CGRID_H
