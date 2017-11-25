#include "CGrid.h"

#include <Qt3DExtras/qcuboidmesh.h>



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
    m_mesh->setGeometry(m_geometry);
    m_mesh->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);

    m_meshRenderer->setGeometry(m_geometry);
    m_meshRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
    addComponent(m_meshRenderer);

    //Custom material
    /*========================================*/
    //Qt3DRender::QMaterial *material1 = new Qt3DRender::QMaterial();
    //Qt3DRender::QMaterial *material2 = new Qt3DRender::QMaterial();

    //// Create effect, technique, render pass and shader
    //Qt3DRender::QEffect *effect = new Qt3DRender::QEffect();
    //Qt3DRender::QTechnique *gl3Technique = new Qt3DRender::QTechnique();
    //Qt3DRender::QRenderPass *gl3Pass = new Qt3DRender::QRenderPass();
    //Qt3DRender::QShaderProgram *glShader = new Qt3DRender::QShaderProgram();

    ////glShader->loadSource(QUrl(":/shaders/vert.glsl"));
    //
    //QFile vertexShaderFile(":shaders/vert2.glsl");
    //QByteArray vertByteArray;
    //if (vertexShaderFile.open(QIODevice::ReadOnly | QIODevice::Text))
    //{
    //    vertByteArray = vertexShaderFile.readAll();
    //}

    //QFile fragmentShaderFile(":shaders/frag.glsl");
    //QByteArray fragByteArray;
    //if (fragmentShaderFile.open(QIODevice::ReadOnly | QIODevice::Text))
    //{
    //    fragByteArray = fragmentShaderFile.readAll();
    //}
    //
    //
    //glShader->setVertexShaderCode(vertByteArray);
    //glShader->setFragmentShaderCode(fragByteArray);        
    //    
    //    // Set the shader on the render pass
    //gl3Pass->setShaderProgram(glShader);

    ////// Add the pass to the technique
    //gl3Technique->addRenderPass(gl3Pass);

    ////// Set the targeted GL version for the technique
   
    //gl3Technique->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::OpenGL);
    //gl3Technique->graphicsApiFilter()->setMajorVersion(3);
    //gl3Technique->graphicsApiFilter()->setMinorVersion(0);
    //gl3Technique->graphicsApiFilter()->setProfile(Qt3DRender::QGraphicsApiFilter::CoreProfile);

    ////// Add the technique to the effect
    //effect->addTechnique(gl3Technique);

    //// Set the effect on the materials
    //material1->setEffect(effect);
    ////material2->setEffect(effect);

    ////// Set different parameters on the materials
    ////const QString parameterName = QStringLiteral("color");
    ////material1->addParameter(new Qt3DRender::QParameter(parameterName, QColor::fromRgbF(0.0f, 1.0f, 0.0f, 1.0f)));
    ////material2->addParameter(new Qt3DRender::QParameter(parameterName, QColor::fromRgbF(1.0f, 1.0f, 1.0f, 1.0f)));

    ////Qt3DRender::QFilterKey* filter = new Qt3DRender::QFilterKey();
    ////filter->setName("renderingStyle");
    ////filter->setValue(QVariant("forward"));
    ////gl3Technique->addFilterKey(filter);


    //addComponent(material1);


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

    QFile vertexShaderFile(":shaders/basic.vert");
    QByteArray vertByteArray;
    if (vertexShaderFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        vertByteArray = vertexShaderFile.readAll();
    }

    QFile fragmentShaderFile(":shaders/basic.frag");
    QByteArray fragByteArray;
    if (fragmentShaderFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        fragByteArray = fragmentShaderFile.readAll();
    }

    //qDebug() << vertByteArray;
    //qDebug() << Qt3DRender::QShaderProgram::loadSource(QUrl(":shaders/frag.glsl"));
    //qDebug() << Qt3DRender::QShaderProgram::loadSource(QUrl(":shaders/basic.frag"));

    glShader->setVertexShaderCode(vertByteArray);
    glShader->setFragmentShaderCode(fragByteArray);

    gl3Technique->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::OpenGL);
    gl3Technique->graphicsApiFilter()->setMajorVersion(3);
    gl3Technique->graphicsApiFilter()->setMinorVersion(1);
    gl3Technique->graphicsApiFilter()->setProfile(Qt3DRender::QGraphicsApiFilter::CoreProfile);

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