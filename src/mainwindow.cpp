#include "mainwindow.h"
#include "ui_mainwindow.h"

// Qt 3D
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qaspectengine.h>

#include <Qt3DRender/qcamera.h>
#include <Qt3DRender/qcameralens.h>
#include <Qt3DRender/qrenderaspect.h>

#include <Qt3DInput/QInputAspect>

#include <Qt3DExtras/qforwardrenderer.h>
#include <Qt3DExtras/qt3dwindow.h>
#include <Qt3DExtras/qfirstpersoncameracontroller.h>


//local includes
#include <CScene.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_pFrameGraph(new FrameGraph())
{
    ui->setupUi(this);
    this->setWindowTitle("GMU Water surface simulation");

    m_mainView = new Qt3DExtras::Qt3DWindow();

    QWidget *container = QWidget::createWindowContainer(m_mainView);

    this->setCentralWidget(container);

    m_scene = new CScene();
    Qt3DCore::QEntity *rootEntity = m_scene->getRootEntity();

    Qt3DInput::QInputAspect *input = new Qt3DInput::QInputAspect;
    m_mainView->registerAspect(input);


    // Scene Camera
    Qt3DRender::QCamera *basicCamera = m_mainView->camera();
    basicCamera->setProjectionType(Qt3DRender::QCameraLens::PerspectiveProjection);

    basicCamera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
    basicCamera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));
    basicCamera->setPosition(QVector3D(0.0f, 0.0f, 30.0f));
    // For camera controls
    Qt3DExtras::QFirstPersonCameraController *camController = new Qt3DExtras::QFirstPersonCameraController(rootEntity);
    camController->setCamera(basicCamera);

    // FrameGraph
    m_mainView->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f)));
    m_mainView->defaultFrameGraph()->setCamera(basicCamera);
    
   // m_pFrameGraph->setCamera(basicCamera);    
   //// rootEntity->addComponent(m_pFrameGraph);


    // Set root object of the scene
    m_mainView->setRootEntity(rootEntity);

    //TODO: Test - delete later
   // m_scene->createSphere();

    this->showMaximized();
}

MainWindow::~MainWindow()
{
}


FrameGraph::FrameGraph(Qt3DCore::QNode* parent)
    : QRenderSettings(parent),
    m_pViewport(new Qt3DRender::QViewport()),
    m_pClearBuffer(new Qt3DRender::QClearBuffers()),
    m_pCameraSelector(new Qt3DRender::QCameraSelector()),
    m_pLayerFilter(new Qt3DRender::QLayerFilter()),
    m_pRenderSurfaceSelector(new Qt3DRender::QRenderSurfaceSelector())
{
    m_pViewport->setNormalizedRect(QRect(0, 0, 1, 1));
    // GONE
    m_pClearBuffer->setClearColor(QColor(QRgb(0x4d4d4f)));
    //setActiveFrameGraph(m_pRenderSurfaceSelector);
    m_pLayerFilter->setEnabled(true);
    m_pLayerFilter->addLayer(new Qt3DRender::QLayer());
    m_pClearBuffer->setBuffers(Qt3DRender::QClearBuffers::AllBuffers);
    //m_pCameraSelector->setParent(m_pClearBuffer);
    m_pCameraSelector->setParent(m_pLayerFilter);

    m_pClearBuffer->setParent(m_pCameraSelector);
    m_pLayerFilter->setParent(m_pRenderSurfaceSelector);
    m_pRenderSurfaceSelector->setParent(m_pViewport);
    //setExternalRenderTargetSize(QSize(800,600));
    // GONE
    setActiveFrameGraph(m_pViewport);
}

FrameGraph::~FrameGraph()
{
    //QNode::cleanup();
}

void FrameGraph::setCamera(Qt3DRender::QCamera* camera)
{
    m_pCameraSelector->setCamera(camera);
}