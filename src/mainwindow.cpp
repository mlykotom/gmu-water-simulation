#include "mainwindow.h"
#include "ui_mainwindow.h"


// Qt 3D
#include <Qt3DRender/qcamera.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DRender/qcameralens.h>

#include <Qt3DInput/QInputAspect>
#include <Qt3DCore/qaspectengine.h>

#include <Qt3DRender/qrenderaspect.h>
#include <Qt3DExtras/qforwardrenderer.h>

#include <Qt3DExtras/qt3dwindow.h>
#include <Qt3DExtras/qfirstpersoncameracontroller.h>

//local includes
#include <CScene.h>
#include <include/CParticleSimulator.h>
#include <include/CParticle.h>

MainWindow::MainWindow(QWidget *parent)
    :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("GMU Water surface simulation");

    Qt3DExtras::Qt3DWindow *m_view = new Qt3DExtras::Qt3DWindow();
    m_view->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f)));
    QWidget *container = QWidget::createWindowContainer(m_view);

    this->setCentralWidget(container);

    Qt3DInput::QInputAspect *input = new Qt3DInput::QInputAspect;
    m_view->registerAspect(input);
    // Camera
    Qt3DRender::QCamera *cameraEntity = m_view->camera();

    cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    cameraEntity->setPosition(QVector3D(0, 0, 20.0f));
    cameraEntity->setUpVector(QVector3D(0, 1, 0));
    cameraEntity->setViewCenter(QVector3D(0, 0, 0));

    m_scene = new CScene();
    Qt3DCore::QEntity *rootEntity = m_scene->getRootEntity();

    // For camera controls
    Qt3DExtras::QFirstPersonCameraController *camController = new Qt3DExtras::QFirstPersonCameraController(rootEntity);
    camController->setCamera(cameraEntity);
    camController->setLookSpeed(camController->lookSpeed() * (-1.0f));

    // Set root object of the scene
    m_view->setRootEntity(rootEntity);

    //TODO: Test - delete later

    auto simulator = new CParticleSimulator(m_scene);

    this->show();
}

MainWindow::~MainWindow()
{
}


