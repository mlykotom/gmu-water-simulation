#include <QOrbitCameraController>
#include <QFirstPersonCameraController>
#include <QTextEdit>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "CLPlatforms.h"

// Qt 3D
#include <QMessageBox>

#include <QCullFace>

//local includes
#include <CQt3DWindow.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("GMU Water surface simulation");

    m_mainView = new CQt3DWindow();
    //m_mainView->setKeyboardGrabEnabled(false);

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
    basicCamera->setPosition(QVector3D(0.0f, 0.0f, 5.0f));
    // For camera controls
    Qt3DExtras::QFirstPersonCameraController * camController = new Qt3DExtras::QFirstPersonCameraController(rootEntity);
    camController->setCamera(basicCamera);

    // FrameGraph
    m_mainView->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f)));
    m_mainView->defaultFrameGraph()->setCamera(basicCamera);


    //Particle simulator
    m_simulator = new CParticleSimulator(m_scene);
    connect(m_mainView, SIGNAL(keyPressed(Qt::Key)), m_simulator, SLOT(onKeyPressed(Qt::Key)));
    connect(m_simulator, &CParticleSimulator::iterationChanged, this, &MainWindow::onSimulationIterationChanged);

    // Set root object of the scene
    m_mainView->setRootEntity(rootEntity);

    try {
        CLPlatforms::printInfoAll();
    }
    catch (CLException &exc) {
        qDebug() << exc.what();
        // TODO alert message?
//        QMessageBox message;
//        message.setText(QString(exc.what()));
//        message.show();
    }

    m_cl_wrapper = new CLWrapper(CLPlatforms::getBestGPU());
    qDebug() << "Selected device: " << CLPlatforms::getDeviceInfo(m_cl_wrapper->getDevice());
}

MainWindow::~MainWindow()
{
//    delete m_scene;
//    delete m_simulator;
    delete m_cl_wrapper;
}

void MainWindow::onSimulationIterationChanged(unsigned long iteration)
{
    double elapsed = m_simulator->getElapsedTime() / 1000.0;
    double fps = iteration / elapsed;
    this->ui->iterationWidget->setText(QString::number(fps));
}

