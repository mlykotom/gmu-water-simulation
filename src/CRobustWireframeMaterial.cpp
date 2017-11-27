#include "CRobustWireframeMaterial.h"

#include <QFile>
#include <QVector4D>
#include <QVector3D>
#include <Qt3DRender/QParameter>

CRobustWireframeMaterial::CRobustWireframeMaterial(Qt3DCore::QNode *parent)
    :QMaterial(parent),
    effect(new Qt3DRender::QEffect()),
    gl3Technique(new Qt3DRender::QTechnique()),
    gl3Pass(new Qt3DRender::QRenderPass()),
    glShader(new Qt3DRender::QShaderProgram())

{

    QFile vertexShaderFile(":shaders/robustwireframe.vert");
    QByteArray vertByteArray;
    if (vertexShaderFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        vertByteArray = vertexShaderFile.readAll();
    }

    QFile fragmentShaderFile(":shaders/robustwireframe.frag");
    QByteArray fragByteArray;
    if (fragmentShaderFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        fragByteArray = fragmentShaderFile.readAll();
    }


    QFile geometryShaderFile(":shaders/robustwireframe.geom");
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

    addParameter(new Qt3DRender::QParameter("ka", QVariant(QVector3D(0.1f, 0.1f, 0.1f))));
    addParameter(new Qt3DRender::QParameter("kd", QVariant(QVector3D(0.7f, 0.7f, 0.7f))));
    addParameter(new Qt3DRender::QParameter("ks", QVariant(QVector3D(0.95f, 0.95f, 0.95f))));
    addParameter(new Qt3DRender::QParameter("shininess", QVariant(150)));

    addParameter(new Qt3DRender::QParameter("light.position", QVariant(QVector4D(0.0f, 0.0f, 0.0f, 1.0f))));
    addParameter(new Qt3DRender::QParameter("light.intensity", QVariant(QVector3D(1.0f, 1.0f, 1.0f))));
    addParameter(new Qt3DRender::QParameter("line.width", QVariant(2.0)));
    addParameter(new Qt3DRender::QParameter("line.color", QVariant(QVector4D(1.0f, 0.0f, 0.0f, 1.0f))));


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

CRobustWireframeMaterial::~CRobustWireframeMaterial()
{

}