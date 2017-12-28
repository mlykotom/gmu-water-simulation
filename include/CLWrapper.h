#ifndef WATERSURFACESIMULATION_CLWRAPPER_H
#define WATERSURFACESIMULATION_CLWRAPPER_H

#include "config.h"
#include <CL/cl.hpp>
#include <fstream>
#include <istream>
#include <QDebug>
#include <utility>


#if PROFILING
#define CL_PROFILING_FLAG CL_QUEUE_PROFILING_ENABLE
#else
#define CL_PROFILING_FLAG 0
#endif

class CLException: public std::runtime_error
{
public:
    explicit CLException(const std::__cxx11::string &__arg) : runtime_error("OpenCL::" + __arg) {}
};

class CLWrapper
{
private:
    cl::Device m_device;
    cl::Context m_context;
    cl::CommandQueue m_queue;
    cl::Program m_program;

    std::string readFile(std::string fileName);

public:
    static const std::string getErrorMessage(cl_int err_id);
    static void checkError(cl_int code, const std::__cxx11::string &codeDescription = "");
    /**
     * Aligns data_size size to align_size
     * @param data
     * @param align_size
     * @return
     */
    static size_t alignTo(size_t data, size_t align_size);
    /**
     * Gets time of event in milliseconds
     * @param event
     * @param waitForEvent
     * @return
     */
    static double getEventDuration(const cl::Event &event, bool waitForEvent = true);
    static double getEventDuration(std::initializer_list<cl::Event> events);

    explicit CLWrapper(cl::Device device);
    virtual ~CLWrapper() = default;

    const cl::Device &getDevice() const { return m_device; }
    const cl::Context &getContext() const { return m_context; }
    const cl::CommandQueue &getQueue() const { return m_queue; }
    const cl::Program &getProgram() const { return m_program; }

    void loadProgram(std::vector<std::string> kernelFiles);
    cl::Kernel getKernel(const std::string &kernelName);

    cl::Buffer createBuffer(cl_mem_flags flags, size_t bufferSize, void *hostPtr = nullptr);

    cl::Event enqueueRead(const cl::Buffer &buffer, size_t size, void *ptr, cl_bool blocking, size_t offset = 0);
    cl::Event enqueueWrite(const cl::Buffer &buffer, size_t size, const void *ptr, cl_bool blocking, size_t offset = 0);
    cl::Event enqueueKernel(cl::Kernel &kernel, const cl::NDRange &global, const cl::NDRange &local = cl::NullRange, const cl::NDRange &offset = cl::NullRange);
};

#endif //WATERSURFACESIMULATION_CLWRAPPER_H