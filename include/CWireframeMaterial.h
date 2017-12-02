#ifndef CWIREFRAMEMATERIAL_H
#define CWIREFRAMEMATERIAL_H

#include <Qt3DRender/QEffect>
#include <Qt3DRender/QRenderPass>
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