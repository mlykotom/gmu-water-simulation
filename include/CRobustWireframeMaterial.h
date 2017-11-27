#ifndef CROBUSTWIREFRAMEMATERIAL_H
#define CROBUSTWIREFRAMEMATERIAL_H

#include <Qt3DRender/QEffect>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QMaterial>

class CRobustWireframeMaterial : public Qt3DRender::QMaterial
{
public:
    CRobustWireframeMaterial(Qt3DCore::QNode *parent = 0);
    ~CRobustWireframeMaterial();

private:
    Qt3DRender::QEffect         *effect;
    Qt3DRender::QTechnique      *gl3Technique;
    Qt3DRender::QRenderPass     *gl3Pass;
    Qt3DRender::QShaderProgram  *glShader;

};

#endif