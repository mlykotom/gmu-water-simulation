#include "CLWrapper.h"

CLWrapper::CLWrapper(cl::Device device) : m_device(std::move(device))
{
    // init context
    cl_int err;
    m_context = cl::Context(m_device, nullptr, nullptr, nullptr, &err);
    checkError(err);

    // init queue
    m_queue = cl::CommandQueue(m_context, m_device, CL_PROFILING_FLAG, &err);
    checkError(err);
}

const std::string CLWrapper::getErrorMessage(cl_int err_id)
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

void CLWrapper::checkError(cl_int code, const std::string &codeDescription)
{
    if (code != CL_SUCCESS && code != CL_DEVICE_NOT_FOUND) {
        auto errorMessage = CLWrapper::getErrorMessage(code);
        throw CLException(errorMessage + "|" + codeDescription);
    }
}

size_t CLWrapper::alignTo(size_t data, size_t align_size)
{
    return ((data - 1 + align_size) / align_size) * align_size;
}

double CLWrapper::getEventDuration(const cl::Event &event, bool waitForEvent)
{
#if PROFILING

    if (waitForEvent) {
        event.wait();
    }

    cl_int err;
    cl_ulong timeStart = event.getProfilingInfo<CL_PROFILING_COMMAND_START>(&err);
    CLWrapper::checkError(err, "CL_PROFILING_COMMAND_START");

    cl_ulong timeEnd = event.getProfilingInfo<CL_PROFILING_COMMAND_END>(&err);
    CLWrapper::checkError(err, "CL_PROFILING_COMMAND_END");
    return double(timeEnd - timeStart) * 1e-6;
#else
    return 0.0;
#endif
}

double CLWrapper::getEventDuration(std::initializer_list<cl::Event> events)
{
#if PROFILING
    cl::Event::waitForEvents(events);
    double duration = 0;

    for (const auto &event : events) {
        duration += getEventDuration(event, false);
    }

    return duration;
#else
    return 0.0;
#endif
}

std::string CLWrapper::readFile(std::string fileName)
{
    auto fin = std::ifstream(fileName, std::ifstream::in | std::ifstream::binary);
    if (fin.bad() || fin.fail()) {
        throw std::runtime_error("Couldn't open source file | readFile");
    }

    std::string content;

    fin.seekg(0, std::ios::end);
    content.reserve(fin.tellg());
    fin.seekg(0, std::ios::beg);

    content.assign((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    return content;
}

void CLWrapper::loadProgram(std::vector<std::string> kernelFiles)
{
    qDebug() << "-> Preparing kernels";

    std::vector<std::string> kernelSourceCodes;
    cl::Program::Sources kernelSources;

    for (const auto &fileName : kernelFiles) {
        kernelSourceCodes.push_back(readFile(fileName));
        kernelSources.emplace_back(kernelSourceCodes.back().c_str(), kernelSourceCodes.back().length());
    }

    cl_int error;
    m_program = cl::Program(m_context, kernelSources, &error);
    checkError(error, "cl::Program creation");

    error = m_program.build(std::vector<cl::Device>(1, m_device));
    qDebug() << "-> Building program";

    if (error == CL_BUILD_PROGRAM_FAILURE) {
        const std::basic_string<char> &buildLog = m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_device, &error);
        const QStringList &list = QString::fromStdString(buildLog).split('\n');
        for (const auto &line : list) {
            qDebug() << line;
        }

        checkError(error, buildLog);
    }
}

cl::Kernel CLWrapper::getKernel(const std::string &kernelName)
{
    cl_int error;
    cl::Kernel kernel(m_program, kernelName.c_str(), &error);
    checkError(error, "cl::Kernel");
    return kernel;
}

cl::Buffer CLWrapper::createBuffer(cl_mem_flags flags, size_t bufferSize, void *hostPtr)
{
    cl_int err;
    auto inputBuffer = cl::Buffer(m_context, flags, bufferSize, hostPtr, &err);
    checkError(err, "inputBuffer creation");
    return inputBuffer;
}

cl::Event CLWrapper::enqueueRead(const cl::Buffer &buffer, size_t size, void *ptr, cl_bool blocking, size_t offset)
{
    cl::Event event;
    cl_int err = m_queue.enqueueReadBuffer(buffer, blocking, offset, size, ptr, nullptr, &event);
    checkError(err, "enqueueReadBuffer");
    return event;
}

cl::Event CLWrapper::enqueueWrite(const cl::Buffer &buffer, size_t size, const void *ptr, cl_bool blocking, size_t offset)
{
    cl::Event event;
    cl_int err = m_queue.enqueueWriteBuffer(buffer, blocking, offset, size, ptr, nullptr, &event);
    checkError(err, "enqueueWriteBuffer");
    return event;
}

cl::Event CLWrapper::enqueueKernel(cl::Kernel &kernel, const cl::NDRange &global, const cl::NDRange &local, const cl::NDRange &offset)
{
    cl::Event event;
    cl_int err = m_queue.enqueueNDRangeKernel(kernel, offset, global, local, nullptr, &event);
    checkError(err, kernel.getInfo<CL_KERNEL_FUNCTION_NAME>());
    return event;
}