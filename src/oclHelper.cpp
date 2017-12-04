#include "oclHelper.h"
#pragma comment( lib, "OpenCL" )

const char *getCLError(cl_int err_id) {
    switch (err_id)
    {
    case CL_SUCCESS:
        return "Success!";
    case CL_DEVICE_NOT_FOUND:
        return "Device not found.";
    case CL_DEVICE_NOT_AVAILABLE:
        return "Device not available";
    case CL_COMPILER_NOT_AVAILABLE:
        return "Compiler not available";
    case CL_MEM_OBJECT_ALLOCATION_FAILURE:
        return "Memory object allocation failure";
    case CL_OUT_OF_RESOURCES:
        return "Out of resources";
    case CL_OUT_OF_HOST_MEMORY:
        return "Out of host memory";
    case CL_PROFILING_INFO_NOT_AVAILABLE:
        return "Profiling information not available";
    case CL_MEM_COPY_OVERLAP:
        return "Memory copy overlap";
    case CL_IMAGE_FORMAT_MISMATCH:
        return "Image format mismatch";
    case CL_IMAGE_FORMAT_NOT_SUPPORTED:
        return "Image format not supported";
    case CL_BUILD_PROGRAM_FAILURE:
        return "Program build failure";
    case CL_MAP_FAILURE:
        return "Map failure";
    case CL_INVALID_VALUE:
        return "Invalid value";
    case CL_INVALID_DEVICE_TYPE:
        return "Invalid device type";
    case CL_INVALID_PLATFORM:
        return "Invalid platform";
    case CL_INVALID_DEVICE:
        return "Invalid device";
    case CL_INVALID_CONTEXT:
        return "Invalid context";
    case CL_INVALID_QUEUE_PROPERTIES:
        return "Invalid queue properties";
    case CL_INVALID_COMMAND_QUEUE:
        return "Invalid command queue";
    case CL_INVALID_HOST_PTR:
        return "Invalid host pointer";
    case CL_INVALID_MEM_OBJECT:
        return "Invalid memory object";
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
        return "Invalid image format descriptor";
    case CL_INVALID_IMAGE_SIZE:
        return "Invalid image size";
    case CL_INVALID_SAMPLER:
        return "Invalid sampler";
    case CL_INVALID_BINARY:
        return "Invalid binary";
    case CL_INVALID_BUILD_OPTIONS:
        return "Invalid build options";
    case CL_INVALID_PROGRAM:
        return "Invalid program";
    case CL_INVALID_PROGRAM_EXECUTABLE:
        return "Invalid program executable";
    case CL_INVALID_KERNEL_NAME:
        return "Invalid kernel name";
    case CL_INVALID_KERNEL_DEFINITION:
        return "Invalid kernel definition";
    case CL_INVALID_KERNEL:
        return "Invalid kernel";
    case CL_INVALID_ARG_INDEX:
        return "Invalid argument index";
    case CL_INVALID_ARG_VALUE:
        return "Invalid argument value";
    case CL_INVALID_ARG_SIZE:
        return "Invalid argument size";
    case CL_INVALID_KERNEL_ARGS:
        return "Invalid kernel arguments";
    case CL_INVALID_WORK_DIMENSION:
        return "Invalid work dimension";
    case CL_INVALID_WORK_GROUP_SIZE:
        return "Invalid work group size";
    case CL_INVALID_WORK_ITEM_SIZE:
        return "Invalid work item size";
    case CL_INVALID_GLOBAL_OFFSET:
        return "Invalid global offset";
    case CL_INVALID_EVENT_WAIT_LIST:
        return "Invalid event wait list";
    case CL_INVALID_EVENT:
        return "Invalid event";
    case CL_INVALID_OPERATION:
        return "Invalid operation";
    case CL_INVALID_GL_OBJECT:
        return "Invalid OpenGL object";
    case CL_INVALID_BUFFER_SIZE:
        return "Invalid buffer size";
    case CL_INVALID_MIP_LEVEL:
        return "Invalid mip-map level";
    default:
        return "Unknown";
    }
}


double getTime(void)
{
  return omp_get_wtime();
}

void clPrintErrorExit(cl_int err_id, const char *msg)
{
    if(err_id != CL_SUCCESS && err_id != CL_DEVICE_NOT_FOUND)
    {
        printf("ERROR code %i: %s\n", err_id, msg);
        system("PAUSE");
        exit(1);
    }
}


char* readFile(const char* filename)
{
    FILE* file;
    if((file = fopen(filename, "rb")) == 0)
    {
        return NULL;
    }

    // ziskame velikost suboru
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // alokace
    char* file_content = (char *)malloc(file_size + 1);
    if (fread(file_content, file_size, 1, file) != 1)
    {
        fclose(file);
        free(file_content);
        return NULL;
    }

    fclose(file);
    file_content[file_size] = '\0';

    return file_content;
}

unsigned int alignTo(unsigned int data, unsigned int align_size)
{
	return ((data - 1 + align_size) / align_size) * align_size;
}

double getEventTime(cl_event i_event)
{
	cl_ulong time_from, time_to;
	clGetEventProfilingInfo(i_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &time_from, NULL);
	clGetEventProfilingInfo(i_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &time_to, NULL);
	return double(time_to - time_from)/1000000000;
}

double getEventTime(cl::Event &event)
{
  return (event.getProfilingInfo<CL_PROFILING_COMMAND_END>() - event.getProfilingInfo<CL_PROFILING_COMMAND_START>()) / 1000000000.0;
}

cl_int *genRandomBuffer(size_t element_count)
{
	srand(omp_get_wtime()*1000000000);
	cl_int *data = (cl_int *)malloc(sizeof(cl_int) * element_count);
	for(int i = 0; i < element_count; i++)
	{
		data[i] = rand();
	}
	return data;
}