#include "mainwindow.h"

#include <oclHelper.h>

#define MATRIX_W 1024
#define MATRIX_H 1024

void matrix_add(cl_int *a, cl_int *b, cl_int *c, int width, int height)
{
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            c[y * width + x] = a[y * width + x] + b[y * width + x];
        }
    }
}

void openCLTest()
{

    // Create host buffers
    cl_int *a_data = genRandomBuffer(MATRIX_W * MATRIX_H);
    cl_int *b_data = genRandomBuffer(MATRIX_W * MATRIX_H);
    cl_int *host_data = (cl_int *) malloc(sizeof(cl_int) * MATRIX_W * MATRIX_H);
    cl_int *device_data = (cl_int *) malloc(sizeof(cl_int) * MATRIX_W * MATRIX_H);

    cl_int err_msg, err_msg2;
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
            qDebug() << QString(" %1. platform name: %2.\n").arg(QString::number(j), platform_devices[j].getInfo<CL_DEVICE_NAME>(&err_msg).c_str());
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

    cl::Device gpu_device;

//    qDebug()<< platforms;

    std::vector<cl::Device> devices;
    platforms.at(0).getDevices(CL_DEVICE_TYPE_GPU, &devices);
    gpu_device = devices.at(1);


    // check if device is correct
    if (gpu_device.getInfo<CL_DEVICE_TYPE>(&err_msg) == CL_DEVICE_TYPE_GPU) {
        printf("\nSelected device type: Correct\n");
    }
    else {
        printf("\nSelected device type: Incorrect\n");
    }
    clPrintErrorExit(err_msg, "cl::Device::getInfo<CL_DEVICE_TYPE>");
    printf("Selected device name: %s.\n", gpu_device.getInfo<CL_DEVICE_NAME>(&err_msg).c_str());
    clPrintErrorExit(err_msg, "cl::Device::getInfo<CL_DEVICE_NAME>");
    platforms.clear();

    //===========================================================================================
    /* ======================================================
    * TODO 2. Cast
    * vytvorit context a query se zapnutym profilovanim
    * =======================================================
    */
    cl_int err;
    cl::Context context(gpu_device, NULL, NULL, NULL, &err);

    if (err) {
        clPrintErrorExit(err, "Context");
    }

    cl::CommandQueue queue(context, gpu_device, CL_QUEUE_PROFILING_ENABLE, &err);

    if (err) {
        clPrintErrorExit(err, "Queue");
    }

    char *program_source = readFile(APP_RESOURCES"/kernels/matrix_add.cl");
    cl::Program::Sources sources;
    sources.push_back(std::pair<const char *, size_t>(program_source, strlen(program_source)));

    // get program
    cl::Program program(context, sources);
    clPrintErrorExit(err_msg, "clCreateProgramWithSource");

    // build program
    if ((err_msg = program.build(std::vector<cl::Device>(1, gpu_device), "", NULL, NULL)) == CL_BUILD_PROGRAM_FAILURE) {
        printf("Build log:\n %s", program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(gpu_device, &err_msg2).c_str());
        clPrintErrorExit(err_msg2, "cl::Program::getBuildInfo<CL_PROGRAM_BUILD_LOG>");
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
    cl::Buffer a_buffer(context, CL_MEM_READ_WRITE, sizeof(cl_int) * MATRIX_W * MATRIX_H, NULL, &err);

    if (err) {
        clPrintErrorExit(err, "Buffer A");

    }

    cl::Buffer b_buffer(context, CL_MEM_READ_WRITE, sizeof(cl_int) * MATRIX_W * MATRIX_H, NULL, &err);

    if (err) {
        clPrintErrorExit(err, "Buffer B");
    }

    cl::Buffer c_buffer(context, CL_MEM_READ_WRITE, sizeof(cl_int) * MATRIX_W * MATRIX_H, NULL, &err);

    if (err) {
        clPrintErrorExit(err, "Buffer C");
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

    cl::UserEvent a_event(context, &err_msg);
    clPrintErrorExit(err_msg, "clCreateUserEvent a_event");
    cl::UserEvent b_event(context, &err_msg);
    clPrintErrorExit(err_msg, "clCreateUserEvent b_event");
    cl::UserEvent kernel_event(context, &err_msg);
    clPrintErrorExit(err_msg, "clCreateUserEvent kernel_event");
    cl::UserEvent c_event(context, &err_msg);
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
    free(a_data);
    free(b_data);
    free(host_data);
    free(device_data);

    return;
}

int main(int argc, char *argv[])
{
    openCLTest();


    QApplication a(argc, argv);

    MainWindow w;
    w.show();


    return a.exec();
}