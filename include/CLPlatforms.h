#ifndef WATERSURFACESIMULATION_CLPLATFORM_H
#define WATERSURFACESIMULATION_CLPLATFORM_H

#include "CLCommon.h"
#include <QDebug>

class CLPlatforms
{
public:
    static std::vector<cl::Platform> getAllPlatforms();
    static std::vector<cl::Device> getDevices(cl::Platform platform, cl_device_type deviceType = CL_DEVICE_TYPE_ALL);

    static void printInfoAll();
    static QString getPlatformInfo(cl::Platform platform, int index = 0);
    static QString getDeviceInfo(cl::Device device, int index = 0);
    static cl::Device getBestGPU();
};


#endif //WATERSURFACESIMULATION_CLPLATFORM_H
