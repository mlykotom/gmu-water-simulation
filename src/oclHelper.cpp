#include "oclHelper.h"
#pragma comment( lib, "OpenCL" )


double getTime(void)
{
    return omp_get_wtime();
}

void clPrintErrorExit(cl_int err_id, const char *msg)
{
    if (err_id != CL_SUCCESS && err_id != CL_DEVICE_NOT_FOUND) {
        printf("ERROR code %i: %s\n", err_id, msg);
        system("PAUSE");
        exit(1);
    }
}

char *readFile(const char *filename)
{
    FILE *file;
    if ((file = fopen(filename, "rb")) == 0) {
        return NULL;
    }

    // ziskame velikost suboru
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // alokace
    char *file_content = (char *) malloc(file_size + 1);
    if (fread(file_content, file_size, 1, file) != 1) {
        fclose(file);
        free(file_content);
        return NULL;
    }

    fclose(file);
    file_content[file_size] = '\0';

    return file_content;
}

double getEventTime(cl_event i_event)
{
    cl_ulong time_from, time_to;
    clGetEventProfilingInfo(i_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &time_from, NULL);
    clGetEventProfilingInfo(i_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &time_to, NULL);
    return double(time_to - time_from) / 1000000000;
}

double getEventTime(cl::Event &event)
{
    return (event.getProfilingInfo<CL_PROFILING_COMMAND_END>() - event.getProfilingInfo<CL_PROFILING_COMMAND_START>()) / 1000000000.0;
}

cl_int *genRandomBuffer(size_t element_count)
{
//	srand(omp_get_wtime()*1000000000);
    qsrand(omp_get_wtime() * 1000000000);

    cl_int *data = (cl_int *) malloc(sizeof(cl_int) * element_count);
    for (int i = 0; i < element_count; i++) {
        data[i] = qrand() / 10000;
    }

    return data;
}