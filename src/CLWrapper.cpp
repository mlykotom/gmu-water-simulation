#include "CLWrapper.h"

std::string CLWrapper::readFile(std::string fileName)
{
    auto fin = std::ifstream(fileName, std::ifstream::in | std::ifstream::binary);
    if (fin.bad() || fin.fail()) {
        throw std::runtime_error("Couldn't open source file");
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

    cl_int errors[2];
    m_program = cl::Program(m_context, kernelSources, errors);
    CLCommon::checkError(errors[0], "cl::Program creation");

    errors[0] = m_program.build(std::vector<cl::Device>(1, m_device));
    qDebug() << "-> Building program";

    if (errors[0] == CL_BUILD_PROGRAM_FAILURE) {
        auto buildLog = m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_device, &errors[1]);
        CLCommon::checkError(errors[1], buildLog);
    }
}

cl::Kernel CLWrapper::getKernel(const std::string &kernelName)
{
    cl_int error;
    cl::Kernel kernel(m_program, kernelName.c_str(), &error);
    CLCommon::checkError(error, "cl::Kernel");
    return kernel;
}