// Qt 3D
#include <QMessageBox>
#include <QCullFace>
#include <QTextEdit>
#include <Qt3DExtras/QFirstPersonCameraController>
#include <QStandardItemModel>
#include "mainwindow.h"
#include "ui_mainwindow.h"

//local includes
#include <CQt3DWindow.h>
#include <CGPUBruteParticleSimulator.h>
#include <CGPUParticleSimulator.h>
#include <CCPUParticleSimulator.h>
#include <CLPlatforms.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    //TODO: move this to setup UI

    ui->setupUi(this);
    this->setWindowTitle("GMU Water surface simulation");
    this->setCentralWidget(this->ui->mainWidget);

    m_mainView = new CQt3DWindow();

    QWidget * container = QWidget::createWindowContainer(m_mainView);
    QSizePolicy centralWidgetSizePolicy = ui->centralWidget->sizePolicy();
    container->setSizePolicy(centralWidgetSizePolicy);
    ui->mainLayout->replaceWidget(ui->centralWidget, container);
    
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

    setupUI();


//    try {
//        //m_simulator = new CCPUParticleSimulator(m_scene);
//        m_simulator = new CCPUBruteParticleSimulator(m_scene);
////        m_simulator = new CGPUParticleSimulator(m_scene);
//
//        connect(this, &MainWindow::keyPressed, m_simulator, &CBaseParticleSimulator::onKeyPressed);
//        connect(m_mainView, &CQt3DWindow::keyPressed, m_simulator, &CBaseParticleSimulator::onKeyPressed);
//        connect(m_simulator, &CBaseParticleSimulator::iterationChanged, this, &MainWindow::onSimulationIterationChanged);
//
//        m_simulator->setupScene();
//        // Set root object of the scene
//        m_mainView->setRootEntity(rootEntity);
//
//    }
//    catch (CLException &exc) {
//        qDebug() << exc.what();
//        QMessageBox message;
//        message.setText(QString(exc.what()));
//        message.exec();
//        exit(1);
//    }
}

MainWindow::~MainWindow()
{
    delete m_simulator;
   // delete m_cl_wrapper;
}

void MainWindow::setupUI()
{
    //ui->particlesCountWidget->setText(QString::number(m_simulator->getParticlesCount()));

    //ui->gridSizeWidget->setText(QString("%1 x %2 x %3").arg(
    //    QString::number(m_simulator->getGridSizeX()),
    //    QString::number(m_simulator->getGridSizeY()),
    //    QString::number(m_simulator->getGridSizeZ())
    //));

  //  ui->selectedDeviceWidget->setText(m_simulator->getSelectedDevice());

    setupDevicesComboBox();
    setupSimulationTypesComboBox();
    
    connect(ui->cubeSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(oncubeSizeSliderValueChanged(int)));
    oncubeSizeSliderValueChanged(ui->cubeSizeSlider->value());
}

void MainWindow::setupDevicesComboBox()
{

    QStringList comboBoxList;

    //platforms
    auto platforms = CLPlatforms::getAllPlatforms();
    unsigned int count = 0;
    QStandardItemModel *comboBoxModel = qobject_cast<QStandardItemModel *>(ui->devicesComboBox->model());

    for (int i = 0; i < platforms.size(); i++)
    {
        ui->devicesComboBox->insertItem(count, "-- " + CLPlatforms::getPlatformInfo(platforms[i]) + " --");
        comboBoxModel->item(count)->setSelectable(false);
        comboBoxModel->item(count)->setBackground(QBrush(QColor(Qt::lightGray)));
        ++count;

        // Devices in that platform
        auto devices = CLPlatforms::getDevices(platforms[i]);
        if (devices.empty()) continue;

        for (int j = 0; j < devices.size(); j++)
        {
            ui->devicesComboBox->insertItem(count, "   " + CLPlatforms::getDeviceInfo(devices[j]));
            ui->devicesComboBox->setItemData(count, i, platformRole);
            ui->devicesComboBox->setItemData(count, j, deviceRole);

            ++count;
        }
    }

    connect(ui->devicesComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDevicesComboBoxIndexChanged(int)));

    //select first device
    for (int i = 0; i < count; ++i)
    {
        if (comboBoxModel->item(i)->isSelectable())
        {
            ui->devicesComboBox->setCurrentIndex(i);
            break;
        }
    }
}

void MainWindow::setupSimulationTypesComboBox()
{
    QStandardItemModel *comboBoxModel = qobject_cast<QStandardItemModel *>(ui->simulationTypeComboBox->model());

    //all available simulation types
    ui->simulationTypeComboBox->insertItem((int)eSimulationType::CPU, "CPU Grid");
    ui->simulationTypeComboBox->setItemData((int)eSimulationType::CPU, (int)eSimulationType::CPU, simulationTypeRole);

    ui->simulationTypeComboBox->insertItem((int)eSimulationType::GPUBrute, "GPU Brute Force");
    ui->simulationTypeComboBox->setItemData((int)eSimulationType::GPUBrute, (int)eSimulationType::GPUBrute, simulationTypeRole);

    ui->simulationTypeComboBox->insertItem((int)eSimulationType::GPUGrid, "GPU Grid");
    ui->simulationTypeComboBox->setItemData((int)eSimulationType::GPUGrid, (int)eSimulationType::GPUGrid, simulationTypeRole);

    connect(ui->simulationTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onSimulationTypeComboBoxIndexChanged(int)));
    onSimulationTypeComboBoxIndexChanged(0);

}

void MainWindow::onDevicesComboBoxIndexChanged(int index)
{
    qDebug() << index;
    qDebug() << ui->devicesComboBox->itemData(index, platformRole);
    qDebug() << ui->devicesComboBox->itemData(index, deviceRole);
}

void MainWindow::onSimulationTypeComboBoxIndexChanged(int index)
{
    //disable devices selection if running CPU simulation
    if (ui->simulationTypeComboBox->itemData(index, simulationTypeRole) == eSimulationType::CPU)
        ui->devicesComboBox->setDisabled(true);
    else
        ui->devicesComboBox->setEnabled(true);

}

void MainWindow::oncubeSizeSliderValueChanged(int value)
{
    ui->cubeSizeLabel->setText(QString::number((float)value / 10.0f, 'g',1));
}

void MainWindow::onSimulationIterationChanged(unsigned long iteration)
{
    //ui->fpsWidget->setText(QString::number(m_simulator->getFps()));
    //ui->iterationWidget->setText(QString::number(iteration));
}
