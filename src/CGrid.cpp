#include "CGrid.h"
#include "CWireframeMaterial.h"
#include "CRobustWireframeMaterial.h"

#include <Qt3DExtras/qcuboidmesh.h>


CGrid::CGrid(Qt3DCore::QNode *parent)
    : RenderableEntity(parent)
    , m_material(new Qt3DExtras::QPhongMaterial())
    , m_meshRenderer(new Qt3DRender::QGeometryRenderer())


{
    //Default Phong Material
    //m_material->setDiffuse(QColor(QRgb(0xa69929)));
    //addComponent(m_material);

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
}
