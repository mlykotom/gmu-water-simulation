#include "CLWrapper.h"

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
    CLCommon::checkError(error, "cl::Program creation");

    error = m_program.build(std::vector<cl::Device>(1, m_device));
    qDebug() << "-> Building program";

    if (error == CL_BUILD_PROGRAM_FAILURE) {
        const std::basic_string<char> &buildLog = m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_device, &error);
        const QStringList &list = QString::fromStdString(buildLog).split('\n');
        for (const auto &line : list) {
            qDebug() << line;
        }

        CLCommon::checkError(error, buildLog);
    }
}

cl::Kernel CLWrapper::getKernel(const std::string &kernelName)
{
    cl_int error;
    cl::Kernel kernel(m_program, kernelName.c_str(), &error);
    CLCommon::checkError(error, "cl::Kernel");
    return kernel;
}

cl::Buffer CLWrapper::createBuffer(cl_mem_flags flags, size_t bufferSize, void *hostPtr)
{
    cl_int err;
    auto inputBuffer = cl::Buffer(m_context, flags, bufferSize, hostPtr, &err);
    CLCommon::checkError(err, "inputBuffer creation");
    return inputBuffer;
}

cl::Event CLWrapper::enqueueRead(const cl::Buffer &buffer, size_t size, void *ptr, cl_bool blocking, size_t offset)
{
    cl::Event event;
    cl_int err = m_queue.enqueueReadBuffer(buffer, blocking, offset, size, ptr, nullptr, &event);
    CLCommon::checkError(err, "enqueueReadBuffer");
    return event;
}

cl::Event CLWrapper::enqueueWrite(const cl::Buffer &buffer, size_t size, const void *ptr, cl_bool blocking, size_t offset)
{
    cl::Event event;
    cl_int err = m_queue.enqueueWriteBuffer(buffer, blocking, offset, size, ptr, nullptr, &event);
    CLCommon::checkError(err, "enqueueWriteBuffer");
    return event;
}

cl::Event CLWrapper::enqueueKernel(cl::Kernel &kernel, const cl::NDRange &global, const cl::NDRange &local, const cl::NDRange &offset)
{
    cl::Event event;
    cl_int err = m_queue.enqueueNDRangeKernel(kernel, offset, global, local, nullptr, &event);
    CLCommon::checkError(err, kernel.getInfo<CL_KERNEL_FUNCTION_NAME>());
    return event;
}
