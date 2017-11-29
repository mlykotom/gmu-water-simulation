#include <QOrbitCameraController>
#include <QFirstPersonCameraController>
#include <QTextEdit>
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

#include <QCullFace>

//local includes
#include <CScene.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("GMU Water surface simulation");

    m_mainView = new Qt3DExtras::Qt3DWindow();

    QWidget *container = QWidget::createWindowContainer(m_mainView);

    this->setCentralWidget(container);

    // Scene
    m_scene = new CScene();
    Qt3DCore::QEntity *rootEntity = m_scene->getRootEntity();
    m_mainView->setRootEntity(rootEntity);


    Qt3DInput::QInputAspect *input = new Qt3DInput::QInputAspect;
    m_mainView->registerAspect(input);


    // Scene Camera
    Qt3DRender::QCamera *basicCamera = m_mainView->camera();
    basicCamera->setProjectionType(Qt3DRender::QCameraLens::PerspectiveProjection);

    basicCamera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
    basicCamera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));
    basicCamera->setPosition(QVector3D(0.0f, 0.0f,5.0f));
    // For camera controls
    Qt3DExtras::QFirstPersonCameraController *camController = new Qt3DExtras::QFirstPersonCameraController(rootEntity);
    camController->setCamera(basicCamera);

    // FrameGraph
    m_mainView->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f)));
    m_mainView->defaultFrameGraph()->setCamera(basicCamera);


    // Set root object of the scene
    m_mainView->setRootEntity(rootEntity);

    //TODO: Test - delete later
   // m_scene->createSphere();
    //m_scene->createScene();

    m_simulator = new CParticleSimulator(m_scene);
    //    m_simulator->start();

    connect(basicCamera, &Qt3DRender::QCamera::viewVectorChanged, this, &MainWindow::onCameraChanged);
    connect(m_simulator, &CParticleSimulator::iterationChanged, this, &MainWindow::onSimulationIterationChanged);

    this->show();
}

MainWindow::~MainWindow()
{
//    delete m_scene;
//    delete m_simulator;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Escape:
            QApplication::quit();
            break;

        case Qt::Key_Space:
            // TODO this is not handled inside of the 3Dwidget :(
            if (m_simulator) {
                m_simulator->toggleSimulation();
            }
            break;

        case Qt::Key_G:
            if (m_simulator) {
                m_simulator->toggleGravity();
            }

            break;
    }
}

void MainWindow::onCameraChanged(const QVector3D &viewVector)
{
//    qDebug() << viewVector;
}
void MainWindow::onSimulationIterationChanged(unsigned long iteration)
{
    this->ui->iterationWidget->setText(QString::number(iteration));
}
