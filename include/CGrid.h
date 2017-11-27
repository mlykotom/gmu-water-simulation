#ifndef CGRID_H
#define CGRID_H

#include "renderableentity.h"
//#include <Qt3DExtras/ge>
#include <qgeometry.h>
//#include <Qt3DExtras/QNormalDiffuseSpecularMapMaterial>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/qcuboidgeometry.h>

class CGrid : RenderableEntity
{
    Q_OBJECT

public:

    CGrid(Qt3DCore::QNode *parent = 0);
    ~CGrid();

private:
    //Qt3DExtras::
    Qt3DExtras::QCuboidGeometry *m_geometry;
    Qt3DExtras::QPhongMaterial *m_material;
    Qt3DRender::QGeometryRenderer *m_meshRenderer;

};

#endif