#ifndef WATERSURFACESIMULATION_CLWRAPPER_H
#define WATERSURFACESIMULATION_CLWRAPPER_H

#include "CLCommon.h"


class CLWrapper
{
private:
    cl::Device m_device;
    cl::Context m_context;
public:
    explicit CLWrapper(cl::Device device)
    {
        m_device = device;

        cl_int err;
        m_context = cl::Context(m_device, nullptr, nullptr, nullptr, &err);
        CLCommon::checkError(err);
    }

    virtual ~CLWrapper()
    {
//        cl::flush();
//        cl::finish();
    }

    const cl::Device &getDevice() const { return m_device; }
    const cl::Context &getContext() const { return m_context; }

    void loadProgram();
};

#endif //WATERSURFACESIMULATION_CLWRAPPER_H
