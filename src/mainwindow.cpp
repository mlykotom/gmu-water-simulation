#include <QOrbitCameraController>
#include <QFirstPersonCameraController>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("GMU Water surface simulation");

    Qt3DExtras::Qt3DWindow *m_view = new Qt3DExtras::Qt3DWindow();
    m_view->defaultFrameGraph()->setClearColor(QColor(QRgb(0xffffff))); //0x4d4d4f
    QWidget *container = QWidget::createWindowContainer(m_view);
    this->setCentralWidget(container);

    Qt3DInput::QInputAspect *input = new Qt3DInput::QInputAspect;
    m_view->registerAspect(input);
    // Camera
    Qt3DRender::QCamera *cameraEntity = m_view->camera();
    cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    cameraEntity->setPosition(QVector3D(0, 0, 15.0f));
    cameraEntity->setUpVector(QVector3D(0, 1, 0));
    cameraEntity->setViewCenter(QVector3D(0, 0, 0));

    // Scene
    m_scene = new CScene();
    Qt3DCore::QEntity *rootEntity = m_scene->getRootEntity();
    m_view->setRootEntity(rootEntity);

    // Set root object of the scene
    // For camera controls
    Qt3DExtras::QFirstPersonCameraController *camController = new Qt3DExtras::QFirstPersonCameraController(rootEntity);
    camController->setLookSpeed(camController->lookSpeed() * (-1.0f));
    camController->setCamera(cameraEntity);

    m_simulator = new CParticleSimulator(m_scene);
    m_simulator->start();

    connect(cameraEntity, &Qt3DRender::QCamera::viewVectorChanged, this, &MainWindow::onCameraChanged);

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

        case Qt::Key_A:
            if (m_simulator) {
            }

            break;
    }
}

void MainWindow::onCameraChanged(const QVector3D &viewVector)
{
//    qDebug() << viewVector;
}