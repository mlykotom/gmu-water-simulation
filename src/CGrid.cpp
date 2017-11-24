#include "CGrid.h"

#include <Qt3DExtras/qcuboidmesh.h>

CGrid::CGrid(Qt3DCore::QNode *parent)
    : RenderableEntity(parent)
    , m_material(new Qt3DExtras::QPhongMaterial())

{
    //Qt3DExtras::QCuboidMesh *cuboid = new Qt3DExtras::QCuboidMesh();
    //addComponent(cuboid);



    m_transform->setScale(4.0f);
    m_transform->setTranslation(QVector3D(5.0f, -4.0f, 0.0f));
    m_material->setDiffuse(QColor(QRgb(0xa69929)));


    m_geometry = new Qt3DExtras::QCuboidGeometry(this);

    m_mesh->setGeometry(m_geometry);

    addComponent(m_mesh);
    addComponent(m_material);
    this->setEnabled(true);

    

}
CGrid::~CGrid()
{
}
