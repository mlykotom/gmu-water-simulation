#include "CGrid.h"
#include "CWireframeMaterial.h"
#include "CRobustWireframeMaterial.h"

#include <Qt3DExtras/qcuboidmesh.h>



CGrid::CGrid(QVector3D size, QVector3D resolution, Qt3DCore::QNode * parent)
    : RenderableEntity(parent),
    m_material(new Qt3DExtras::QPhongMaterial()),
    m_meshRenderer(new Qt3DRender::QGeometryRenderer()),
    m_ResX(resolution.x()),
    m_ResY(resolution.y()),
    m_ResZ(resolution.z())
{
    m_cell_count = (m_ResX * m_ResY * m_ResZ);
    m_data = new std::vector<CParticle *>[m_cell_count];

    //Cuboid geometry
    m_geometry = new Qt3DExtras::QCuboidGeometry(this);
    m_geometry->setZExtent(size.z());
    m_geometry->setYExtent(size.y());
    m_geometry->setXExtent(size.x());

    m_geometry->updateVertices();
    m_geometry->updateIndices();


    m_meshRenderer->setGeometry(m_geometry);
    m_meshRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
    addComponent(m_meshRenderer);

    //collision geometry
    m_collisionGeometry = new CCollisionGeometry(m_geometry);

    //Custom material
    CWireframeMaterial *wireframeMaterial = new CWireframeMaterial();
    addComponent(wireframeMaterial);

    this->setEnabled(true);

}


CGrid::CGrid(Qt3DCore::QNode *parent)
    : RenderableEntity(parent),
    m_material(new Qt3DExtras::QPhongMaterial()),
    m_meshRenderer(new Qt3DRender::QGeometryRenderer()),
    m_ResX(0),
    m_ResY(0),
    m_ResZ(0),
    m_cell_count(0)
{
    //Translation
    m_transform->setScale(4.0f);
    m_transform->setTranslation(QVector3D(0, 0, -10.0f));

    //Cuboid geometry
    m_geometry = new Qt3DExtras::QCuboidGeometry(this);
    m_geometry->updateVertices();
    m_geometry->updateIndices();

    m_meshRenderer->setGeometry(m_geometry);
    m_meshRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
    addComponent(m_meshRenderer);


    /*============================ Working area ====================================================*/
    //collision geometry
    m_collisionGeometry = new CCollisionGeometry(m_geometry);




    /*============================ Working area ====================================================*/


    //Custom material
    CWireframeMaterial *wireframeMaterial = new CWireframeMaterial();
    addComponent(wireframeMaterial);

    //CRobustWireframeMaterial *robustWireframeMaterial = new CRobustWireframeMaterial();
    //addComponent(robustWireframeMaterial);

    this->setEnabled(true);
}
CGrid::~CGrid()
{
    delete[] m_data;
}