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

//custom material
#include <QEffect>
#include <QRenderPass>
#include <QTechnique>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QMaterial>

class CWireframeMaterial : public Qt3DRender::QMaterial
{
public:
    CWireframeMaterial(Qt3DCore::QNode *parent = 0);
    ~CWireframeMaterial();

private:
    Qt3DRender::QEffect         *effect;
    Qt3DRender::QTechnique      *gl3Technique;
    Qt3DRender::QRenderPass     *gl3Pass;
    Qt3DRender::QShaderProgram  *glShader;

};
#endif