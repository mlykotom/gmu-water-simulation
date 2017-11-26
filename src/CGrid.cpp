#include "CGrid.h"

#include <Qt3DExtras/qcuboidmesh.h>

#include <QCullFace>
#include <QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QBufferDataGeneratorPtr>


//
#include <QFile>

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
    CWireframeMaterial *mat = new CWireframeMaterial();
    addComponent(mat);

    /*================================================================*/



    this->setEnabled(true);
}
CGrid::~CGrid()
{
}


CWireframeMaterial::CWireframeMaterial(Qt3DCore::QNode *parent )
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

    //qDebug() << vertByteArray;
    //qDebug() << Qt3DRender::QShaderProgram::loadSource(QUrl(":shaders/frag.glsl"));
    //qDebug() << Qt3DRender::QShaderProgram::loadSource(QUrl(":shaders/basic.frag"));

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


    //Qt3DRender::QCullFace *cull = new Qt3DRender::QCullFace();
    //cull->setMode(Qt3DRender::QCullFace::NoCulling);
    //gl3Pass->addRenderState(cull);

    setEffect(effect);
    setEnabled(true);
}

CWireframeMaterial::~CWireframeMaterial()
{

}