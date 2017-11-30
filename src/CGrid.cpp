#include "CGrid.h"
#include "CWireframeMaterial.h"
#include "CRobustWireframeMaterial.h"

#include <Qt3DExtras/qcuboidmesh.h>


CGrid::CGrid(int x, int y, int z, Qt3DCore::QNode * parent)
    : RenderableEntity(parent),
    m_material(new Qt3DExtras::QPhongMaterial()),
    m_meshRenderer(new Qt3DRender::QGeometryRenderer()),
    m_x(x), 
    m_y(y),
    m_z(z), 
    m_cell_count(x * y * z)
{
    m_data = new std::vector<CParticle *>[m_cell_count];

    //Cuboid geometry
    m_geometry = new Qt3DExtras::QCuboidGeometry(this);
    m_geometry->setZExtent(z);
    m_geometry->setYExtent(y);
    m_geometry->setXExtent(x);

    m_meshRenderer->setGeometry(m_geometry);
    m_meshRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
    addComponent(m_meshRenderer);

    //Custom material
    /*========================================*/
    CWireframeMaterial *wireframeMaterial = new CWireframeMaterial();
    addComponent(wireframeMaterial);

    this->setEnabled(true);

}


CGrid::CGrid(Qt3DCore::QNode *parent)
    : RenderableEntity(parent),
    m_material(new Qt3DExtras::QPhongMaterial()),
    m_meshRenderer(new Qt3DRender::QGeometryRenderer()),
    m_x(0),
    m_y(0),
    m_z(0),
    m_cell_count(0)
{
    //Translation
    m_transform->setScale(4.0f);
    m_transform->setTranslation(QVector3D(5.0f, -4.0f, 0.0f)); 

    //Cuboid geometry
    m_geometry = new Qt3DExtras::QCuboidGeometry(this);

    m_meshRenderer->setGeometry(m_geometry);
    m_meshRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
    addComponent(m_meshRenderer);

    //Custom material
    /*========================================*/
    CWireframeMaterial *wireframeMaterial = new CWireframeMaterial();
    addComponent(wireframeMaterial);

    //CRobustWireframeMaterial *robustWireframeMaterial = new CRobustWireframeMaterial();
    //addComponent(robustWireframeMaterial);

    /*================================================================*/
    this->setEnabled(true);
}
CGrid::~CGrid()
{
    //delete[] m_data;
}

std::vector<CParticle*> CGrid::getNeighborsCells(int x, int y, int z)
{
    {
        auto neighborParticles = std::vector<CParticle *>();

        for (int offsetX = -1; offsetX <= 1; offsetX++) {
            if (x + offsetX < 0) continue;
            if (x + offsetX >= xRes()) break;

            for (int offsetY = -1; offsetY <= 1; offsetY++) {
                if (y + offsetY < 0) continue;
                if (y + offsetY >= yRes()) break;

                for (int offsetZ = -1; offsetZ <= 1; offsetZ++) {
                    if (z + offsetZ < 0) continue;
                    if (z + offsetZ >= zRes()) break;

                    auto &particlesAtCell = at(x + offsetX, y + offsetY, z + offsetZ);
                    neighborParticles.insert(neighborParticles.end(), particlesAtCell.begin(), particlesAtCell.end());
                }
            }
        }

        return neighborParticles;
    }
}
