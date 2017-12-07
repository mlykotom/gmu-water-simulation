#include <CL/cl.hpp>
#include <qplatformdefs.h>
#include <include/CLCommon.h>
#include <include/CLWrapper.h>
#include <include/mainwindow.h>
#include "CLPlatforms.h"

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

cl_int *genRandomBuffer(size_t element_count)
{
    qsrand(CLCommon::getTime() * 1000000000);

    cl_int *data = new cl_int[element_count];
    for (int i = 0; i < element_count; i++) {
        data[i] = qrand() / 10000;
    }

    return data;
}

void doCalculation()
{
    auto m_cl_wrapper = new CLWrapper(CLPlatforms::getBestGPU());


//    m_cl_wrapper->loadProgram(
//        {
//            APP_RESOURCES"/kernels/test_matrix_add.cl"
//        }
//    );

    cl_int err_msg;

    cl::Buffer a_buffer(m_cl_wrapper->getContext(), CL_MEM_READ_WRITE, sizeof(cl_int) * MATRIX_W * MATRIX_H, NULL, &err_msg);

    if (err_msg) {
        CLCommon::checkError(err_msg, "Buffer A");
    }

    cl::Buffer b_buffer(m_cl_wrapper->getContext(), CL_MEM_READ_WRITE, sizeof(cl_int) * MATRIX_W * MATRIX_H, NULL, &err_msg);

    if (err_msg) {
        CLCommon::checkError(err_msg, "Buffer B");
    }

    cl::Buffer c_buffer(m_cl_wrapper->getContext(), CL_MEM_READ_WRITE, sizeof(cl_int) * MATRIX_W * MATRIX_H, NULL, &err_msg);

    if (err_msg) {
        CLCommon::checkError(err_msg, "Buffer C");
    }

    cl_int matrix_width = MATRIX_W;
    cl_int matrix_height = MATRIX_H;

    //===========================================================================================
    /* ======================================================
    * TODO 4. Cast
    * nastavit parametry spusteni
    * =======================================================
    */

    cl::Kernel kernel = m_cl_wrapper->getKernel("matrix_add");

    kernel.setArg(0, a_buffer);
    kernel.setArg(1, b_buffer);
    kernel.setArg(2, c_buffer);
    kernel.setArg(3, matrix_width);
    kernel.setArg(4, matrix_height);

    cl::UserEvent a_event(m_cl_wrapper->getContext(), &err_msg);
    CLCommon::checkError(err_msg, "clCreateUserEvent a_event");
    cl::UserEvent b_event(m_cl_wrapper->getContext(), &err_msg);
    CLCommon::checkError(err_msg, "clCreateUserEvent b_event");
    cl::UserEvent kernel_event(m_cl_wrapper->getContext(), &err_msg);
    CLCommon::checkError(err_msg, "clCreateUserEvent kernel_event");
    cl::UserEvent c_event(m_cl_wrapper->getContext(), &err_msg);
    CLCommon::checkError(err_msg, "clCreateUserEvent c_event");

    //===========================================================================================
    /* ======================================================
    * TODO 5. Cast
    * velikost skupiny, kopirovat data na gpu, spusteni kernelu, kopirovani dat zpet
    * pro zarovnání muzete pouzit funkci alignTo(co, na_nasobek_ceho)
    * jako vystupni event kopirovani nastavte prepripravene eventy a_event b_event c_event
    * vystupni event kernelu kernel_event
    * =======================================================
    */

    double gpu_start = CLCommon::getTime();

    cl::NDRange local(16, 16);
    cl::NDRange global(CLCommon::alignTo(MATRIX_W, 16), CLCommon::alignTo(MATRIX_H, 16));

    // Create host buffers
    cl_int *a_data = genRandomBuffer(MATRIX_W * MATRIX_H);
    cl_int *b_data = genRandomBuffer(MATRIX_W * MATRIX_H);
    cl_int *host_data = new cl_int[MATRIX_W * MATRIX_H]();
    cl_int *device_data = new cl_int[MATRIX_W * MATRIX_H]();

    m_cl_wrapper->getQueue().enqueueWriteBuffer(a_buffer, false, 0, sizeof(cl_int) * MATRIX_W * MATRIX_H, a_data, nullptr, &a_event);
    m_cl_wrapper->getQueue().enqueueWriteBuffer(b_buffer, false, 0, sizeof(cl_int) * MATRIX_W * MATRIX_H, b_data, nullptr, &b_event);
    m_cl_wrapper->getQueue().enqueueNDRangeKernel(kernel, 0, global, local, nullptr, &kernel_event);
    m_cl_wrapper->getQueue().enqueueReadBuffer(c_buffer, false, 0, sizeof(cl_int) * MATRIX_W * MATRIX_H, device_data, nullptr, &c_event);

    // synchronize queue
    CLCommon::checkError(m_cl_wrapper->getQueue().finish(), "clFinish");

    double gpu_end = CLCommon::getTime();

    // compute results on host
    double cpu_start = CLCommon::getTime();
    matrix_add(a_data, b_data, host_data, MATRIX_W, MATRIX_H);
    double cpu_end = CLCommon::getTime();

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
    printf(" OpenCL copy time: %fs\n", CLCommon::getEventTime(a_event) + CLCommon::getEventTime(b_event) + CLCommon::getEventTime(c_event));
    printf(" OpenCL processing time: %fs\n", CLCommon::getEventTime(kernel_event));

    // deallocate host data
    m_cl_wrapper->getQueue().flush();
    m_cl_wrapper->getQueue().finish();

    CLCommon::checkError(cl::flush(), "error");
    CLCommon::checkError(cl::finish(), "error");

    delete[] a_data;
    delete[] b_data;
    delete[] host_data;
    delete[] device_data;

    delete (m_cl_wrapper);
}
