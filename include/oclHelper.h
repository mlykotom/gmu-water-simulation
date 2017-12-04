#ifndef OCL_HELPER_H
#define OCL_HELPER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#include <CL/cl.hpp>

// convert opencl error code to string
const char *getCLError(cl_int err_id);

// get time in seconds
double getTime(void);

// check errors and exit
void clPrintErrorExit(cl_int err_id, const char *msg);

// Read file to string
char* readFile(const char* filename);

// align data_size size to align_size
unsigned int alignTo(unsigned int data_size, unsigned int align_size);

// get event time
double getEventTime(cl_event i_event);
double getEventTime(cl::Event &i_event);

// generate random buffer
cl_int *genRandomBuffer(size_t element_count);

#endif
