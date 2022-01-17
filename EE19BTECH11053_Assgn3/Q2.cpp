*
 * Copyright (C) 2020-2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <level_zero/zes_api.h>

#include <algorithm>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <map>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

bool verbose = true;

std::string getErrorString(ze_result_t error)
{
    static const std::map<ze_result_t, std::string> mgetErrorString{
        {ZE_RESULT_NOT_READY, "ZE_RESULT_NOT_READY"},
        {ZE_RESULT_ERROR_DEVICE_LOST, "ZE_RESULT_ERROR_DEVICE_LOST"},
        {ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY, "ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY"},
        {ZE_RESULT_ERROR_OUT_OF_DEVICE_MEMORY, "ZE_RESULT_ERROR_OUT_OF_DEVICE_MEMORY"},
        {ZE_RESULT_ERROR_MODULE_BUILD_FAILURE, "ZE_RESULT_ERROR_MODULE_BUILD_FAILURE"},
        {ZE_RESULT_ERROR_MODULE_LINK_FAILURE, "ZE_RESULT_ERROR_MODULE_LINK_FAILURE"},
        {ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS, "ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS"},
        {ZE_RESULT_ERROR_NOT_AVAILABLE, "ZE_RESULT_ERROR_NOT_AVAILABLE"},
        {ZE_RESULT_ERROR_DEPENDENCY_UNAVAILABLE, "ZE_RESULT_ERROR_DEPENDENCY_UNAVAILABLE"},
        {ZE_RESULT_ERROR_UNINITIALIZED, "ZE_RESULT_ERROR_UNINITIALIZED"},
        {ZE_RESULT_ERROR_UNSUPPORTED_VERSION, "ZE_RESULT_ERROR_UNSUPPORTED_VERSION"},
        {ZE_RESULT_ERROR_UNSUPPORTED_FEATURE, "ZE_RESULT_ERROR_UNSUPPORTED_FEATURE"},
        {ZE_RESULT_ERROR_INVALID_ARGUMENT, "ZE_RESULT_ERROR_INVALID_ARGUMENT"},
        {ZE_RESULT_ERROR_INVALID_NULL_HANDLE, "ZE_RESULT_ERROR_INVALID_NULL_HANDLE"},
        {ZE_RESULT_ERROR_HANDLE_OBJECT_IN_USE, "ZE_RESULT_ERROR_HANDLE_OBJECT_IN_USE"},
        {ZE_RESULT_ERROR_INVALID_NULL_POINTER, "ZE_RESULT_ERROR_INVALID_NULL_POINTER"},
        {ZE_RESULT_ERROR_INVALID_SIZE, "ZE_RESULT_ERROR_INVALID_SIZE"},
        {ZE_RESULT_ERROR_UNSUPPORTED_SIZE, "ZE_RESULT_ERROR_UNSUPPORTED_SIZE"},
        {ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT, "ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT"},
        {ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT, "ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT"},
        {ZE_RESULT_ERROR_INVALID_ENUMERATION, "ZE_RESULT_ERROR_INVALID_ENUMERATION"},
        {ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION, "ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION"},
        {ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT, "ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT"},
        {ZE_RESULT_ERROR_INVALID_NATIVE_BINARY, "ZE_RESULT_ERROR_INVALID_NATIVE_BINARY"},
        {ZE_RESULT_ERROR_INVALID_GLOBAL_NAME, "ZE_RESULT_ERROR_INVALID_GLOBAL_NAME"},
        {ZE_RESULT_ERROR_INVALID_KERNEL_NAME, "ZE_RESULT_ERROR_INVALID_KERNEL_NAME"},
        {ZE_RESULT_ERROR_INVALID_FUNCTION_NAME, "ZE_RESULT_ERROR_INVALID_FUNCTION_NAME"},
        {ZE_RESULT_ERROR_INVALID_GROUP_SIZE_DIMENSION, "ZE_RESULT_ERROR_INVALID_GROUP_SIZE_DIMENSION"},
        {ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION, "ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION"},
        {ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX, "ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX"},
        {ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE, "ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE"},
        {ZE_RESULT_ERROR_INVALID_KERNEL_ATTRIBUTE_VALUE, "ZE_RESULT_ERROR_INVALID_KERNEL_ATTRIBUTE_VALUE"},
        {ZE_RESULT_ERROR_INVALID_MODULE_UNLINKED, "ZE_RESULT_ERROR_INVALID_MODULE_UNLINKED"},
        {ZE_RESULT_ERROR_INVALID_COMMAND_LIST_TYPE, "ZE_RESULT_ERROR_INVALID_COMMAND_LIST_TYPE"},
        {ZE_RESULT_ERROR_OVERLAPPING_REGIONS, "ZE_RESULT_ERROR_OVERLAPPING_REGIONS"},
        {ZE_RESULT_ERROR_UNKNOWN, "ZE_RESULT_ERROR_UNKNOWN"}};
    auto i = mgetErrorString.find(error);
    if (i == mgetErrorString.end())
        return "ZE_RESULT_ERROR_UNKNOWN";
    else
        return mgetErrorString.at(error);
}

#define VALIDATECALL(myZeCall)                \
    do                                        \
    {                                         \
        ze_result_t r = myZeCall;             \
        if (r != ZE_RESULT_SUCCESS)           \
        {                                     \
            std::cout << getErrorString(r)    \
                      << " returned by "      \
                      << #myZeCall << ": "    \
                      << __FUNCTION__ << ": " \
                      << __LINE__ << "\n";    \
        }                                     \
    } while (0);

void getDeviceHandles(ze_driver_handle_t &driverHandle, std::vector<ze_device_handle_t> &devices, int argc, char *argv[])
{

    VALIDATECALL(zeInit(ZE_INIT_FLAG_GPU_ONLY));

    uint32_t driverCount = 0;
    VALIDATECALL(zeDriverGet(&driverCount, nullptr));
    if (driverCount == 0)
    {
        std::cout << "Error could not retrieve driver" << std::endl;
        std::terminate();
    }
    VALIDATECALL(zeDriverGet(&driverCount, &driverHandle));

    uint32_t deviceCount = 0;
    VALIDATECALL(zeDeviceGet(driverHandle, &deviceCount, nullptr));
    if (deviceCount == 0)
    {
        std::cout << "Error could not retrieve device" << std::endl;
        std::terminate();
    }
    devices.resize(deviceCount);
    VALIDATECALL(zeDeviceGet(driverHandle, &deviceCount, devices.data()));

    ze_device_properties_t deviceProperties = {ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES};
    for (const auto &device : devices)
    {
        VALIDATECALL(zeDeviceGetProperties(device, &deviceProperties));

        if (verbose)
        {
            std::cout << "Device Name = " << deviceProperties.name << std::endl;
            std::cout << "deviceProperties.flags =  " << deviceProperties.flags << "on device" << device << std::endl;
        }
    }
}
//After getting the device handles
//Let us use this Temperature_Monitor function using the mentioned 3 APIs to complete the task
void Temperature_Monitor(std::vector<ze_device_handle_t> &devices) 
{   
    std::cout << std::endl;

    uint32_t numSens = 0;
//We use the zesDeviceEnumTemperatureSensors API to get handle of the temperature sensors
    VALIDATECALL(zesDeviceEnumTemperatureSensors(devices, &numSens, NULL));
    if (numSens == 0)
    {
        std::cout << "Error: Could not get handle of temp. sensors" << std::endl;
    }
//We use the Temp. properties and state api structures here where we check the properties and monitor the state
    zes_temp_properties_t temp;
    for (const auto &temp_properties) 
    {    
        float t_i = 0;
        float t_p = 0;

        VALIDATECALL(zesTemperatureGetProperties(temp, &temp_properties));
        if (verbose) 
        {
           std::cout << "maxTemp "<< temp_properties.maxTemperature;
           //maxTemperature is the only double value supported, gives output in deg. Celcius
           std::cout<<std::endl;
        }

        VALIDATECALL(zesTemperatureGetState(temp, &t_i));
        if (verbose)
        {
            std::cout << "instantaneous_temp" << t_i << std::endl;
        }
	    float t_d = t_p - t_i;
        if (verbose) 
        {
            std::cout << 10 << std::endl;  //Temp Data freq.
        }
        t_d = 0;
        while(1)
        {
        VALIDATECALL(zesTemperatureGetState(temp, &t_p));     //Ongoing loop for continuous checking and monitoring   
        }
    }
}
bool validateGetenv(const char *name)
{
    const char *env = getenv(name);
    if ((nullptr == env) || (0 == strcmp("0", env)))
        return false;
    return (0 == strcmp("1", env));
}
int main(int argc, char *argv[])
{
    std::vector<ze_device_handle_t> devices;
    ze_driver_handle_t driver;

    if (!validateGetenv("ZES_ENABLE_SYSMAN"))
    {
        std::cout << "Must set environment variable ZES_ENABLE_SYSMAN=1" << std::endl;
        exit(0);
    }
    getDeviceHandles(driver, devices, argc, argv);
    Temperature_Monitor(devices[0]);
}