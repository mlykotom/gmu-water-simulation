#include "mainwindow.h"
#include "ui_mainwindow.h"

// Qt 3D
#include <Qt3DCore/qentity.h>
#include <Qt3DRender/qcameralens.h>

#include <Qt3DInput/QInputAspect>
#include <Qt3DCore/qaspectengine.h>

#include <Qt3DRender/qrenderaspect.h>
#include <Qt3DExtras/qforwardrenderer.h>

#include <Qt3DExtras/qt3dwindow.h>
#include <Qt3DRender/qcamera.h>
#include <Qt3DExtras/qfirstpersoncameracontroller.h>

//local includes
#include <CScene.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("GMU Water surface simulation");

    Qt3DExtras::Qt3DWindow *m_mainView = new Qt3DExtras::Qt3DWindow();
    m_mainView->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f)));
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

    // Set root object of the scene
    m_mainView->setRootEntity(rootEntity);

    //TODO: Test - delete later
   // m_scene->createSphere();

    this->showMaximized();
}

MainWindow::~MainWindow()
{
}
