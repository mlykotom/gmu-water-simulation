#ifndef OCL_HELPER_H
#define OCL_HELPER_H

#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <QtGlobal>
#include <omp.h>

#include <CL/cl.hpp>

// get time in seconds
double getTime(void);

// check errors and exit
void clPrintErrorExit(cl_int err_id, const char *msg);

// Read file to string
char *readFile(const char *filename);

// get event time
double getEventTime(cl_event i_event);
double getEventTime(cl::Event &i_event);

// generate random buffer
cl_int *genRandomBuffer(size_t element_count);

#endif
