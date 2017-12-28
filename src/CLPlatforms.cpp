#include "CLPlatforms.h"
#include "CLWrapper.h"

std::vector<cl::Platform> CLPlatforms::getAllPlatforms()
{
    std::vector<cl::Platform> platforms;
    int error = cl::Platform::get(&platforms);
    CLWrapper::checkError(error);
    return platforms;
}

std::vector<cl::Device> CLPlatforms::getDevices(cl::Platform platform, cl_device_type deviceType)
{
    std::vector<cl::Device> devices;
    int error = platform.getDevices(deviceType, &devices);
    CLWrapper::checkError(error);

    return devices;
}

/**
 * Print platform name
 * @param platform
 */
QString CLPlatforms::getPlatformInfo(const cl::Platform &platform)
{
    cl_int err;
    auto platformInfo = platform.getInfo<CL_PLATFORM_NAME>(&err);
    CLWrapper::checkError(err, "cl::Platform::getInfo<CL_PLATFORM_NAME>");
    return QString(platformInfo.c_str());
}

/**
 * Get device name
 * @param device
 */
QString CLPlatforms::getDeviceInfo(const cl::Device &device)
{
    cl_int err;
    auto deviceInfo = device.getInfo<CL_DEVICE_NAME>(&err);
    CLWrapper::checkError(err, "cl::Device::getInfo<CL_DEVICE_NAME>");
    auto maxWorkGroupSize = device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>(&err);
    CLWrapper::checkError(err, "cl::Device::getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>");
    return QString("%1 (%2)").arg(QString(deviceInfo.c_str()), QString::number(maxWorkGroupSize));
}

void CLPlatforms::printInfoAll()
{
    qDebug() << "Platforms:";
    qDebug() << "------------------";

    auto platforms = getAllPlatforms();

    for (int i = 0; i < platforms.size(); i++) {
        qDebug() << i << getPlatformInfo(platforms[i]);

        // Get platform devices count
        auto devices = getDevices(platforms[i]);
        if (devices.empty()) continue;

        for (int j = 0; j < devices.size(); j++) {
            qDebug() << "\t" << j << getDeviceInfo(devices[j]);
        }
    }
    qDebug() << "------------------";
}

/**
  * TODO find based on vendor / name or something
 * @return
 */
QPair<int, int> CLPlatforms::getBestGPUIndices()
{
    int platform_index, device_index;
#ifdef __APPLE__
    platform_index = 0;
//    device_index = 0; // cpu
//    device_index = 1 ; //intel gpu
    device_index = 2; //amd gpu
#else
    ////nvidia GPU
    platform_index = 1;
    device_index = 0;

    //intel CPU
    //platform_index = 0;
    //device_index = 1;
#endif

    return QPair<int, int>(platform_index, device_index);
};

cl::Device CLPlatforms::getBestGPU()
{
    cl_device_type device_type = CL_DEVICE_TYPE_ALL;

    const QPair<int, int> &indices = getBestGPUIndices();

    int platform_index = indices.first;
    int device_index = indices.second;

    cl::Platform platform = getAllPlatforms()[platform_index];
    return getDevices(platform, device_type)[device_index];
}