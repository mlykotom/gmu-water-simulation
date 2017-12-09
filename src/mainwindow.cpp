// Qt 3D
#include <QMessageBox>
#include <QCullFace>
#include <QTextEdit>
#include <Qt3DExtras/QFirstPersonCameraController>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ocl_test.h"

//local includes
#include <CQt3DWindow.h>
#include <include/CCPUBruteParticleSimulator.h>
#include <include/CGPUParticleSimulator.h>

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
    Qt3DExtras::QFirstPersonCameraController *camController = new Qt3DExtras::QFirstPersonCameraController(rootEntity);
    camController->setCamera(basicCamera);

    // FrameGraph
    m_mainView->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f)));
    m_mainView->defaultFrameGraph()->setCamera(basicCamera);

//    doCalculation();


    try {
        //Particle simulator
//        m_simulator = new CCPUParticleS imulator(m_scene, nullptr);
        m_simulator = new CGPUParticleSimulator(m_scene);
        //m_simulator = new CCPUBruteParticleSimulator(m_scene);

//        m_simulator = new CCPUParticleSimulator(m_scene, nullptr);
//        m_simulator = new CGPUParticleSimulator(m_scene);
       // m_simulator = new CCPUBruteParticleSimulator(m_scene);
		m_simulator = new CGPUParticleSimulator(m_scene);

        
        connect(m_mainView, &CQt3DWindow::keyPressed, m_simulator, &CBaseParticleSimulator::onKeyPressed);
        connect(m_simulator, &CBaseParticleSimulator::iterationChanged, this, &MainWindow::onSimulationIterationChanged);

        //m_simulator->setupScene();
		m_simulator->test();
        // Set root object of the scene
        m_mainView->setRootEntity(rootEntity);
    }
    catch (CLException &exc) {
        qDebug() << exc.what();
        QMessageBox message;
        message.setText(QString(exc.what()));
        message.exec();
        exit(1);
    }
}

MainWindow::~MainWindow()
{
//    delete m_scene;
    delete m_simulator;
    delete m_cl_wrapper;
}

void MainWindow::onSimulationIterationChanged(unsigned long iteration)
{
    QString text = QString("%1 | %2").arg(
        QString::number(m_simulator->getParticlesCount()),
        QString::number(m_simulator->getFps())
    );

    this->ui->iterationWidget->setText(text);
}

