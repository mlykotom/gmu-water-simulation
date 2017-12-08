#ifndef WATERSURFACESIMULATION_CLWRAPPER_H
#define WATERSURFACESIMULATION_CLWRAPPER_H

#include "CLCommon.h"
#include <fstream>
#include <QDebug>

class CLWrapper
{
private:
    cl::Device m_device;
    cl::Context m_context;
    cl::CommandQueue m_queue;
    cl::Program m_program;

    std::string readFile(std::string fileName);

public:
    explicit CLWrapper(cl::Device device)
    {
        // init device
        m_device = std::move(device);

        // init context
        cl_int err;
        m_context = cl::Context(m_device, nullptr, nullptr, nullptr, &err);
        CLCommon::checkError(err);

        // init queue
        m_queue = cl::CommandQueue(m_context, m_device, CL_QUEUE_PROFILING_ENABLE, &err);
        CLCommon::checkError(err);
    }

    virtual ~CLWrapper()
    {
//        cl::finish();
    }

    const cl::Device &getDevice() const { return m_device; }
    const cl::Context &getContext() const { return m_context; }
    const cl::CommandQueue &getQueue() const { return m_queue; }
    const cl::Program &getProgram() const { return m_program; }

    void loadProgram(std::vector<std::string> kernelFiles);
    cl::Kernel getKernel(const std::string &kernelName);

    cl::Buffer createBuffer(cl_mem_flags flags, size_t bufferSize, void *hostPtr = nullptr);
};

#endif //WATERSURFACESIMULATION_CLWRAPPER_H
