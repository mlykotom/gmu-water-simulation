#include <QOrbitCameraController>
#include <QFirstPersonCameraController>
#include <QTextEdit>
#include "mainwindow.h"
#include "ui_mainwindow.h"

// Qt 3D

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

    createCLContext();

    doCalculation();

}

MainWindow::~MainWindow()
{
//    delete m_scene;
//    delete m_simulator;
    clPrintErrorExit(cl::flush(), "error");
    clPrintErrorExit(cl::finish(), "error");

}

void MainWindow::createCLContext()
{
    cl_int err_msg;
    std::vector<cl::Platform> platforms;
    std::vector<cl::Device> platform_devices;
    // Get Platforms count
    clPrintErrorExit(cl::Platform::get(&platforms), "cl::Platform::get");

    qDebug() << "Platforms:\n";
    for (int i = 0; i < platforms.size(); i++) {
        // Print platform name
        qDebug() << QString(" %1. platform name: %2.\n").arg(QString::number(i), platforms[i].getInfo<CL_PLATFORM_NAME>(&err_msg).c_str());
        //printf(" %d. platform name: %s.\n", i, platforms[i].getInfo<CL_PLATFORM_NAME>(&err_msg).c_str());
        clPrintErrorExit(err_msg, "cl::Platform::getInfo<CL_PLATFORM_NAME>");

        // Get platform devices count
        clPrintErrorExit(platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &platform_devices), "getDevices");
        if (platform_devices.size() == 0) continue;

        for (int j = 0; j < platform_devices.size(); j++) {
            // Get device name
            qDebug() << QString("  %1. device name: %2.\n").arg(QString::number(j), platform_devices[j].getInfo<CL_DEVICE_NAME>(&err_msg).c_str());
            //  printf("  %d. device name: %s.\n", j, platform_devices[j].getInfo<CL_DEVICE_NAME>(&err_msg).c_str());
            clPrintErrorExit(err_msg, "cl::Device::getInfo<CL_DEVICE_NAME>");
        }
        platform_devices.clear();
    }

    //===========================================================================================
    /* ======================================================
    * TODO 1. Cast
    * ziskat gpu device
    * =======================================================
    */

    std::vector<cl::Device> devices;

    int platform_index, device_index;

#ifdef __APPLE__
    platform_index = 0;
    device_index = 1;
#else
    platform_index = 1;
    device_index = 0;
#endif

    platforms.at(platform_index).getDevices(CL_DEVICE_TYPE_GPU, &devices);
    m_gpu_device = devices.at(device_index);

    qDebug() << "Selected device: " << m_gpu_device.getInfo<CL_DEVICE_NAME>(&err_msg).c_str();

    // check if device is correct
    if (m_gpu_device.getInfo<CL_DEVICE_TYPE>(&err_msg) == CL_DEVICE_TYPE_GPU) {
        printf("\nSelected device type: Correct\n");
    }
    else {
        printf("\nSelected device type: Incorrect\n");
    }
    clPrintErrorExit(err_msg, "cl::Device::getInfo<CL_DEVICE_TYPE>");
    printf("Selected device name: %s.\n", m_gpu_device.getInfo<CL_DEVICE_NAME>(&err_msg).c_str());
    clPrintErrorExit(err_msg, "cl::Device::getInfo<CL_DEVICE_NAME>");
    platforms.clear();

    //===========================================================================================
    /* ======================================================
    * TODO 2. Cast
    * vytvorit context a query se zapnutym profilovanim
    * =======================================================
    */
    cl_int err;
    m_context = cl::Context(m_gpu_device, NULL, NULL, NULL, &err);

    if (err) {
        clPrintErrorExit(err, "Context");
    }
}

void matrix_add(cl_int *a, cl_int *b, cl_int *c, int width, int height)
{
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            c[y * width + x] = a[y * width + x] + b[y * width + x];
        }
    }
}

void MainWindow::doCalculation()
{
    //===========================================================================================
    /* ======================================================
    * TODO 2. Cast
    * vytvorit context a query se zapnutym profilovanim
    * =======================================================
    */
    cl_int err_msg;

    cl::CommandQueue queue(m_context, m_gpu_device, CL_QUEUE_PROFILING_ENABLE, &err_msg);

    if (err_msg) {
        clPrintErrorExit(err_msg, "Queue");
    }

    char *program_source = readFile(APP_RESOURCES"/kernels/matrix_add.cl");
    cl::Program::Sources sources;
    sources.push_back(std::pair<const char *, size_t>(program_source, strlen(program_source)));

    // get program
    cl::Program program(m_context, sources);
    clPrintErrorExit(err_msg, "clCreateProgramWithSource");

    // build program
    if ((err_msg = program.build(std::vector<cl::Device>(1, m_gpu_device), "", NULL, NULL)) == CL_BUILD_PROGRAM_FAILURE) {
        printf("Build log:\n %s", program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_gpu_device, &err_msg).c_str());
        clPrintErrorExit(err_msg, "cl::Program::getBuildInfo<CL_PROGRAM_BUILD_LOG>");
    }
    clPrintErrorExit(err_msg, "clBuildProgram");

    // create kernel
    cl::Kernel kernel(program, "matrix_add", &err_msg);
    clPrintErrorExit(err_msg, "cl::Kernel");

    //===========================================================================================
    /* ======================================================
    * TODO 3. Cast
    * vytvorit buffery
    * =======================================================
    */
    cl::Buffer a_buffer(m_context, CL_MEM_READ_WRITE, sizeof(cl_int) * MATRIX_W * MATRIX_H, NULL, &err_msg);

    if (err_msg) {
        clPrintErrorExit(err_msg, "Buffer A");

    }

    cl::Buffer b_buffer(m_context, CL_MEM_READ_WRITE, sizeof(cl_int) * MATRIX_W * MATRIX_H, NULL, &err_msg);

    if (err_msg) {
        clPrintErrorExit(err_msg, "Buffer B");
    }

    cl::Buffer c_buffer(m_context, CL_MEM_READ_WRITE, sizeof(cl_int) * MATRIX_W * MATRIX_H, NULL, &err_msg);

    if (err_msg) {
        clPrintErrorExit(err_msg, "Buffer C");
    }


    cl_int matrix_width = MATRIX_W;
    cl_int matrix_height = MATRIX_H;

    //===========================================================================================
    /* ======================================================
    * TODO 4. Cast
    * nastavit parametry spusteni
    * =======================================================
    */

    kernel.setArg(0, a_buffer);
    kernel.setArg(1, b_buffer);
    kernel.setArg(2, c_buffer);
    kernel.setArg(3, matrix_width);
    kernel.setArg(4, matrix_height);

    cl::UserEvent a_event(m_context, &err_msg);
    clPrintErrorExit(err_msg, "clCreateUserEvent a_event");
    cl::UserEvent b_event(m_context, &err_msg);
    clPrintErrorExit(err_msg, "clCreateUserEvent b_event");
    cl::UserEvent kernel_event(m_context, &err_msg);
    clPrintErrorExit(err_msg, "clCreateUserEvent kernel_event");
    cl::UserEvent c_event(m_context, &err_msg);
    clPrintErrorExit(err_msg, "clCreateUserEvent c_event");


    //===========================================================================================
    /* ======================================================
    * TODO 5. Cast
    * velikost skupiny, kopirovat data na gpu, spusteni kernelu, kopirovani dat zpet
    * pro zarovnání muzete pouzit funkci alignTo(co, na_nasobek_ceho)
    * jako vystupni event kopirovani nastavte prepripravene eventy a_event b_event c_event
    * vystupni event kernelu kernel_event
    * =======================================================
    */

    double gpu_start = getTime();

    cl::NDRange local(16, 16);
    cl::NDRange global(alignTo(MATRIX_W, 16), alignTo(MATRIX_H, 16));

    // Create host buffers
    cl_int *a_data = genRandomBuffer(MATRIX_W * MATRIX_H);
    cl_int *b_data = genRandomBuffer(MATRIX_W * MATRIX_H);
    cl_int *host_data = (cl_int *) malloc(sizeof(cl_int) * MATRIX_W * MATRIX_H);
    cl_int *device_data = (cl_int *) malloc(sizeof(cl_int) * MATRIX_W * MATRIX_H);

    queue.enqueueWriteBuffer(a_buffer, false, 0, sizeof(cl_int) * MATRIX_W * MATRIX_H, a_data, NULL, &a_event);
    queue.enqueueWriteBuffer(b_buffer, false, 0, sizeof(cl_int) * MATRIX_W * MATRIX_H, b_data, NULL, &b_event);

    queue.enqueueNDRangeKernel(kernel, 0, global, local, NULL, &kernel_event);

    queue.enqueueReadBuffer(c_buffer, false, 0, sizeof(cl_int) * MATRIX_W * MATRIX_H, device_data, NULL, &c_event);

    // synchronize queue
    clPrintErrorExit(queue.finish(), "clFinish");

    double gpu_end = getTime();

    // compute results on host
    double cpu_start = getTime();
    matrix_add(a_data, b_data, host_data, MATRIX_W, MATRIX_H);
    double cpu_end = getTime();

    // check data
    if (memcmp(device_data, host_data, MATRIX_W * MATRIX_H * sizeof(cl_int)) == 0) {
        printf("\nResult: Correct\n");
    }
    else {
        printf("\nResult: Incorrect\n");
    }

    // print results
    printf("\nExample results:\n");
    for (int x = 0; x < 10; x++) {
        int y = x + 1;
        int i = y * MATRIX_W + x;
        printf(" [%d,%d] %d + %d = %d(gpu) %d(cpu)\n", y, x, a_data[i], b_data[i], device_data[i], host_data[i]);
    }

    // print performance info
    printf("\nHost timers:\n");
    printf(" OpenCL processing time: %fs\n", gpu_end - gpu_start);
    printf(" CPU    processing time: %fs\n", cpu_end - cpu_start);
    printf("\nDevice timers:\n");
    printf(" OpenCL copy time: %fs\n", getEventTime(a_event) + getEventTime(b_event) + getEventTime(c_event));
    printf(" OpenCL processing time: %fs\n", getEventTime(kernel_event));

    // deallocate host data
    queue.flush();
    queue.finish();


    clPrintErrorExit(cl::flush(), "error");
    clPrintErrorExit(cl::finish(), "error");


    free(a_data);
    free(b_data);
    free(host_data);
    free(device_data);

    return;
}

void MainWindow::onSimulationIterationChanged(unsigned long iteration)
{
    this->ui->iterationWidget->setText(QString::number(iteration));
}

