#include "CLPlatforms.h"

std::vector<cl::Platform> CLPlatforms::getAllPlatforms()
{
    std::vector<cl::Platform> platforms;
    int error = cl::Platform::get(&platforms);
    CLCommon::checkError(error);
    return platforms;
}

std::vector<cl::Device> CLPlatforms::getDevices(cl::Platform platform, cl_device_type deviceType)
{
    std::vector<cl::Device> devices;
    int error = platform.getDevices(deviceType, &devices);
    CLCommon::checkError(error);

    return devices;
}

/**
 * Print platform name
 * @param platform
 */
QString CLPlatforms::getPlatformInfo(cl::Platform platform, int index)
{
    cl_int err;
    auto platformInfo = platform.getInfo<CL_PLATFORM_NAME>(&err);
    CLCommon::checkError(err, "cl::Platform::getInfo<CL_PLATFORM_NAME>");
    return QString("%1. platform name: %2").arg(QString::number(index), platformInfo.c_str());
}

/**
 * Get device name
 * @param device
 */
QString CLPlatforms::getDeviceInfo(cl::Device device, int index)
{
    cl_int err;
    auto deviceInfo = device.getInfo<CL_DEVICE_NAME>(&err);
    CLCommon::checkError(err, "cl::Device::getInfo<CL_DEVICE_NAME>");
    return QString("  %1. device name: %2.").arg(QString::number(index), deviceInfo.c_str());
}

void CLPlatforms::printInfoAll()
{
    qDebug() << "Platforms:";
    qDebug() << "------------------";

    auto platforms = getAllPlatforms();

    for (int i = 0; i < platforms.size(); i++) {
        qDebug() << getPlatformInfo(platforms[i], i);

        // Get platform devices count
        auto devices = getDevices(platforms[i]);
        if (devices.empty()) continue;

        for (int j = 0; j < devices.size(); j++) {
            qDebug() << getDeviceInfo(devices[j], j);
        }
    }
    qDebug() << "------------------";
}

/**
 * TODO find based on vendor / name or something
 * @return
 */
cl::Device CLPlatforms::getBestGPU()
{
    int platform_index, device_index, device_type;
#ifdef __APPLE__
    platform_index = 0;
//    device_index = 0; // cpu
//    device_index = 1 ; //intel gpu
    device_index = 2 ; //amd gpu
    device_type = CL_DEVICE_TYPE_ALL;
#else
    ////nvidia GPU
    //platform_index = 1;
    //device_index = 0;

    //intel CPU
    platform_index = 0;
    device_index = 1;
    device_type = CL_DEVICE_TYPE_ALL;
#endif

    cl::Platform platform = getAllPlatforms()[platform_index];

    return getDevices(platform, device_type)[device_index];
}