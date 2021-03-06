#ifndef WATERSURFACESIMULATION_CLPLATFORM_H
#define WATERSURFACESIMULATION_CLPLATFORM_H

#include "CLWrapper.h"
#include <QDebug>

class CLPlatforms
{
public:
    static std::vector<cl::Platform> getAllPlatforms();
    static std::vector<cl::Device> getDevices(cl::Platform platform, cl_device_type deviceType = CL_DEVICE_TYPE_ALL);

    static void printInfoAll();
    static QString getPlatformInfo(const cl::Platform &platform);
    static QString getDeviceInfo(const cl::Device &device);
    static QPair<int, int> getBestGPUIndices();
    static cl::Device getBestGPU();
};


#endif //WATERSURFACESIMULATION_CLPLATFORM_H
