//local includes
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_simulator(nullptr),
    m_scene(nullptr),
    m_mainView(nullptr),
    m_simulationIsReady(false)
{
    setupUI();
    showMaximized();
}

MainWindow::~MainWindow()
{
    delete m_simulator;
}

void MainWindow::setupUI()
{
    ui->setupUi(this);
    this->setWindowTitle("GMU Water surface simulation");
    this->setCentralWidget(this->ui->mainWidget);

    setup3DWidget();
    setupScene();
    setupDevicesComboBox();
    setupSimulationTypesComboBox();

    connect(ui->cubeSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(onCubeSizeSliderValueChanged(int)));
    onCubeSizeSliderValueChanged(ui->cubeSizeSlider->value());

    connect(ui->setupPushButton, SIGNAL(clicked()), this, SLOT(onSetupSimulationClicked()));
    connect(ui->startPushButton, SIGNAL(clicked()), this, SLOT(onStartSimulationClicked()));
    connect(ui->pausePushButton, SIGNAL(clicked()), this, SLOT(onPauseSimulationClicked()));
    connect(ui->stopPushButton, SIGNAL(clicked()), this, SLOT(onStopSimulationClicked()));

    togglePushButtons(false);
}

void MainWindow::setupDevicesComboBox()
{

    QStringList comboBoxList;

    //platforms
    auto platforms = CLPlatforms::getAllPlatforms();
    unsigned int count = 0;
    QStandardItemModel *comboBoxModel = qobject_cast<QStandardItemModel *>(ui->devicesComboBox->model());

    for (int i = 0; i < platforms.size(); i++) {
        ui->devicesComboBox->insertItem(count, "-- " + CLPlatforms::getPlatformInfo(platforms[i]) + " --");
        comboBoxModel->item(count)->setSelectable(false);
        comboBoxModel->item(count)->setBackground(QBrush(QColor(Qt::lightGray)));
        ++count;

        // Devices in that platform
        auto devices = CLPlatforms::getDevices(platforms[i]);
        if (devices.empty()) continue;

        for (int j = 0; j < devices.size(); j++) {
            ui->devicesComboBox->insertItem(count, "   " + CLPlatforms::getDeviceInfo(devices[j]));
            ui->devicesComboBox->setItemData(count, i, platformRole);
            ui->devicesComboBox->setItemData(count, j, deviceRole);

            ++count;
        }
    }

    connect(ui->devicesComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDevicesComboBoxIndexChanged(int)));

    //select first device
    for (int i = 0; i < count; ++i) {
        if (comboBoxModel->item(i)->isSelectable()) {
            ui->devicesComboBox->setCurrentIndex(i);
            break;
        }
    }
}

void MainWindow::setupSimulationTypesComboBox()
{
    QStandardItemModel *comboBoxModel = qobject_cast<QStandardItemModel *>(ui->simulationTypeComboBox->model());

    //all available simulation types
    ui->simulationTypeComboBox->insertItem((int) eSimulationType::GPUGrid, "GPU Grid");
    ui->simulationTypeComboBox->setItemData((int) eSimulationType::GPUGrid, (int) eSimulationType::GPUGrid, simulationTypeRole);

    ui->simulationTypeComboBox->insertItem((int) eSimulationType::GPUBrute, "GPU Brute Force");
    ui->simulationTypeComboBox->setItemData((int) eSimulationType::GPUBrute, (int) eSimulationType::GPUBrute, simulationTypeRole);

    ui->simulationTypeComboBox->insertItem((int) eSimulationType::CPU, "CPU Grid");
    ui->simulationTypeComboBox->setItemData((int) eSimulationType::CPU, (int) eSimulationType::CPU, simulationTypeRole);

    connect(ui->simulationTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onSimulationTypeComboBoxIndexChanged(int)));
    onSimulationTypeComboBoxIndexChanged(0);

    QStandardItemModel *scenarioComboBox = qobject_cast<QStandardItemModel *>(ui->scenarioComboBox->model());
    ui->scenarioComboBox->insertItem((int) SimulationScenario::DAM_BREAK, "Dam break");
    ui->scenarioComboBox->setItemData((int) SimulationScenario::DAM_BREAK, (int) SimulationScenario::DAM_BREAK);

    ui->scenarioComboBox->insertItem((int) SimulationScenario::FOUNTAIN, "Fountain");
    ui->scenarioComboBox->setItemData((int) SimulationScenario::FOUNTAIN, (int) SimulationScenario::FOUNTAIN);
}

void MainWindow::setup3DWidget()
{
    m_mainView = new CQt3DWindow();
    QWidget * container = QWidget::createWindowContainer(m_mainView);
    QSizePolicy centralWidgetSizePolicy = ui->centralWidget->sizePolicy();
    container->setSizePolicy(centralWidgetSizePolicy);
    ui->mainLayout->replaceWidget(ui->centralWidget, container);

    Qt3DInput::QInputAspect *input = new Qt3DInput::QInputAspect;
    m_mainView->registerAspect(input);

    // Scene Camera
    Qt3DRender::QCamera *basicCamera = m_mainView->camera();
    basicCamera->setProjectionType(Qt3DRender::QCameraLens::PerspectiveProjection);
    resetCamera(basicCamera);

    // FrameGraph
    m_mainView->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f)));
    m_mainView->defaultFrameGraph()->setCamera(basicCamera);
}

void MainWindow::resetCamera(Qt3DRender::QCamera *pCamera)
{
    pCamera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
    pCamera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));
    pCamera->setPosition(QVector3D(0.0f, 0.0f, 4.0f));
}

void MainWindow::setupScene()
{
    // Scene
    m_scene = new CScene();
    Qt3DCore::QEntity *rootEntity = m_scene->getRootEntity();
    m_mainView->setRootEntity(rootEntity);

    // Scene Camera
    Qt3DRender::QCamera *basicCamera = m_mainView->camera();

    // For camera controls
    Qt3DExtras::QOrbitCameraController * camController = new Qt3DExtras::QOrbitCameraController(rootEntity);
    camController->setCamera(basicCamera);
    camController->setLinearSpeed(4.0);
}

void MainWindow::resetScene()
{
    if (m_simulator != nullptr) {
        m_simulator->stop();
        delete m_simulator;
        m_simulator = nullptr;
    }

    if (m_scene != nullptr) {
        m_mainView->setRootEntity(nullptr);

        delete m_scene;
        m_scene = nullptr;
    }
    setupScene();
}

void MainWindow::createSimulator()
{
    if (m_simulator != nullptr) {
        m_simulator->stop();
        delete m_simulator;
    }

    cl::Device device = CLPlatforms::getDevices(CLPlatforms::getAllPlatforms()[m_simulationOptions.platformIndex], CL_DEVICE_TYPE_ALL)[m_simulationOptions.deviceIndex];

    auto scenario = (SimulationScenario) ui->scenarioComboBox->currentData().toInt();

    switch (m_simulationOptions.type) {
        case eSimulationType::CPU:
            m_simulator = new CCPUParticleSimulator(m_scene, m_simulationOptions.boxSize, scenario);
            break;
        case eSimulationType::GPUBrute:
            m_simulator = new CGPUBruteParticleSimulator(m_scene, m_simulationOptions.boxSize, device, scenario);
            break;
        case eSimulationType::GPUGrid:
            m_simulator = new CGPUParticleSimulator(m_scene, m_simulationOptions.boxSize, device, scenario);
            break;
        default:
            break;
    }

    connect(this, &MainWindow::keyPressed, m_simulator, &CBaseParticleSimulator::onKeyPressed);
    connect(m_mainView, &CQt3DWindow::keyPressed, m_simulator, &CBaseParticleSimulator::onKeyPressed);
    connect(m_mainView, &CQt3DWindow::keyPressed, this, &MainWindow::onKeyPressed);
    connect(m_simulator, &CBaseParticleSimulator::iterationChanged, this, &MainWindow::onSimulationIterationChanged);
    connect(m_simulator, &CBaseParticleSimulator::errorOccured, this, &MainWindow::onError);
}

void MainWindow::togglePushButtons(bool value)
{
    ui->startPushButton->setEnabled(!value);
    ui->setupPushButton->setEnabled(!value);

    ui->stopPushButton->setEnabled(value);
    ui->pausePushButton->setEnabled(value);
}

void MainWindow::onDevicesComboBoxIndexChanged(int index)
{
    m_simulationOptions.platformIndex = ui->devicesComboBox->itemData(index, platformRole).toInt();
    m_simulationOptions.deviceIndex = ui->devicesComboBox->itemData(index, deviceRole).toInt();
    m_simulationIsReady = false;

}

void MainWindow::onSimulationTypeComboBoxIndexChanged(int index)
{
    m_simulationOptions.type = (eSimulationType) ui->simulationTypeComboBox->itemData(index, simulationTypeRole).toInt();

    //disable devices selection if running CPU simulation
    if (m_simulationOptions.type == eSimulationType::CPU)
        ui->devicesComboBox->setDisabled(true);
    else
        ui->devicesComboBox->setEnabled(true);

    m_simulationIsReady = false;

}

void MainWindow::onCubeSizeSliderValueChanged(int value)
{
    m_simulationOptions.boxSize = (float) value / 10.0;
    ui->cubeSizeLabel->setText(QString::number(m_simulationOptions.boxSize, 'g', 1));

    m_simulationIsReady = false;

}

void MainWindow::onStartSimulationClicked()
{
    if (!m_simulationIsReady)
        onSetupSimulationClicked();

    togglePushButtons(true);

    m_simulator->start();
}

void MainWindow::onPauseSimulationClicked()
{
    if (m_simulator != nullptr)
        m_simulator->toggleSimulation();
}

void MainWindow::onStopSimulationClicked()
{
    exportLogs();
    resetScene();
    m_simulationIsReady = false;

    ui->FPSLabel->clear();
    ui->iterationWidget->clear();

    togglePushButtons(false);
}

void MainWindow::onSetupSimulationClicked()
{
    if (!m_simulationIsReady) {
        try {
            resetScene();
            createSimulator();
            m_simulator->setupScene();

            ui->particlesNumberLabel->setText(QString::number(m_simulator->getParticlesCount()));

            m_simulationIsReady = true;
        }
        catch (CLException &exc) {
            onError(exc.what());
        }
    }
}

void MainWindow::onSimulationIterationChanged(unsigned long iteration)
{
    if (m_simulator->getMaxParticlesCount() != m_simulator->getParticlesCount()) {
        ui->particlesNumberLabel->setText(QString::number(m_simulator->getParticlesCount()) + " | " + QString::number(m_simulator->getMaxParticlesCount()));
    }
    else {
        // for static number of particles
        ui->particlesNumberLabel->setText(QString::number(m_simulator->getParticlesCount()));
    }

    ui->FPSLabel->setText(QString::number(m_simulator->getFps(), 'f', 2));
    ui->iterationWidget->setText(QString::number(iteration));
}

void MainWindow::onError(const char *error)
{
    qDebug() << error;
    QMessageBox message;
    message.setText(QString(error));
    message.exec();
}
void MainWindow::exportLogs()
{
    QString fileName = QString("%1_%2").arg(
        QString::number(ui->cubeSizeSlider->value() / 10.0),
        ui->devicesComboBox->currentText()
    );

    QFile data("../logs/" + fileName + ".csv");

    if (data.open(QFile::WriteOnly | QFile::Append)) {
        QTextStream output(&data);

        QString fps = "";
        QTextStream fpsStream(&fps);

        for (auto pair : m_simulator->fpsEvents) {
            fpsStream << pair.second << ';';
        }

        output << ui->simulationTypeComboBox->currentText() << ';';
        output << fps;
        output << '\n';
    }
}

void MainWindow::onKeyPressed(Qt::Key key)
{
    switch (key) {
        case Qt::Key_R:
            resetCamera(m_mainView->camera());
            break;
    }
}