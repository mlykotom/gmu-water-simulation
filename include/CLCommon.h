#ifndef WATERSURFACESIMULATION_CLCOMMON_H
#define WATERSURFACESIMULATION_CLCOMMON_H

#include <string>
#include <exception>
#include <stdexcept>
#include <CL/cl.hpp>


class CLException: public std::runtime_error
{
public:
    explicit CLException(const std::string &__arg) : runtime_error("OpenCL::" + __arg) {}
};

class CLCommon
{
public:
    static const std::string getErrorMessage(cl_int err_id)
    {
        switch (err_id) {
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

    static void checkError(cl_int code, const std::string &codeDescription = "")
    {
        if (code != CL_SUCCESS && code != CL_DEVICE_NOT_FOUND) {
            auto errorMessage = getErrorMessage(code);
            throw CLException(errorMessage + "|" + codeDescription);
        }
    }

    /**
     * Aligns data_size size to align_size
     * @param data
     * @param align_size
     * @return
     */
    static inline size_t alignTo(size_t data, size_t align_size)
    {
        return ((data - 1 + align_size) / align_size) * align_size;
    }

    /**
     * Gets time of event in milliseconds
     * @param event
     * @param waitForEvent
     * @return
     */
    static double getEventDuration(const cl::Event &event, bool waitForEvent = true)
    {
        if (waitForEvent) {
            event.wait();
        }

        cl_int err;
        cl_ulong timeStart = event.getProfilingInfo<CL_PROFILING_COMMAND_START>(&err);
        CLCommon::checkError(err, "CL_PROFILING_COMMAND_START");

        cl_ulong timeEnd = event.getProfilingInfo<CL_PROFILING_COMMAND_END>(&err);
        CLCommon::checkError(err, "CL_PROFILING_COMMAND_END");
        return double(timeEnd - timeStart) * 1e-6;
    }

    static double getEventDuration(std::initializer_list<cl::Event> events)
    {
        cl::Event::waitForEvents(events);
        double duration = 0;

        for (const auto &event : events) {
            duration += getEventDuration(event, false);
        }

        return duration;
    }
};

#endif //WATERSURFACESIMULATION_CLCOMMON_H
