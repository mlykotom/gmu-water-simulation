#include "CWireframeMaterial.h"

#include <QFile>
#include <QVector4D>
#include <Qt3DRender/QParameter>

CWireframeMaterial::CWireframeMaterial(Qt3DCore::QNode *parent)
    :QMaterial(parent),
    effect(new Qt3DRender::QEffect()),
    gl3Technique(new Qt3DRender::QTechnique()),
    gl3Pass(new Qt3DRender::QRenderPass()),
    glShader(new Qt3DRender::QShaderProgram())

{

    QFile vertexShaderFile(":shaders/wireframe.vert");
    QByteArray vertByteArray;
    if (vertexShaderFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        vertByteArray = vertexShaderFile.readAll();
    }

    QFile fragmentShaderFile(":shaders/wireframe.frag");
    QByteArray fragByteArray;
    if (fragmentShaderFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        fragByteArray = fragmentShaderFile.readAll();
    }


    QFile geometryShaderFile(":shaders/wireframe.geom");
    QByteArray geomByteArray;
    if (geometryShaderFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        geomByteArray = geometryShaderFile.readAll();
    }


    glShader->setVertexShaderCode(vertByteArray);
    glShader->setFragmentShaderCode(fragByteArray);
    glShader->setGeometryShaderCode(geomByteArray);

    gl3Technique->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::OpenGL);
    gl3Technique->graphicsApiFilter()->setMajorVersion(3);
    gl3Technique->graphicsApiFilter()->setMinorVersion(1);
    gl3Technique->graphicsApiFilter()->setProfile(Qt3DRender::QGraphicsApiFilter::CoreProfile);

    //addParameter(new Qt3DRender::QParameter("line.width", QVariant(2.0)));
    addParameter(new Qt3DRender::QParameter("lineColor", QVariant(QVector4D(0.0f, 0.0f, 0.0f, 1.0f))));

    Qt3DRender::QFilterKey* filter = new Qt3DRender::QFilterKey();
    filter->setName("renderingStyle");
    filter->setValue(QVariant("forward"));
    gl3Technique->addFilterKey(filter);

    gl3Pass->setShaderProgram(glShader);

    gl3Technique->addRenderPass(gl3Pass);

    effect->addTechnique(gl3Technique);

    setEffect(effect);
    setEnabled(true);
}

CWireframeMaterial::~CWireframeMaterial()
{

}