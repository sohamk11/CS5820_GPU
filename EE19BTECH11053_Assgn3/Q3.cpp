/*
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

void testSysmanFrequency(ze_device_handle_t &device) {
    std::cout << std::endl
              << " ----  Frequency tests ---- " << std::endl;
    bool iamroot = (geteuid() == 0);

    uint32_t count = 0;
    VALIDATECALL(zesDeviceEnumFrequencyDomains(device, &count, nullptr));
    if (count == 0) {
        std::cout << "Could not retrieve frequency domains" << std::endl;
        return;
    }
    std::vector<zes_freq_handle_t> handles(count, nullptr);
    VALIDATECALL(zesDeviceEnumFrequencyDomains(device, &count, handles.data()));

    for (const auto &handle : handles) {
        zes_freq_properties_t freqProperties = {};
        zes_freq_range_t freqRange = {};
        zes_freq_range_t testFreqRange = {};
        zes_freq_state_t freqState = {};

        VALIDATECALL(zesFrequencyGetProperties(handle, &freqProperties));
        if (verbose) {
            std::cout << "freqProperties.type = " << freqProperties.type << std::endl;
            std::cout << "freqProperties.canControl = " << freqProperties.canControl << std::endl;
            std::cout << "freqProperties.isThrottleEventSupported = " << freqProperties.isThrottleEventSupported << std::endl;
            std::cout << "freqProperties.min = " << freqProperties.min << std::endl;
            std::cout << "freqProperties.max = " << freqProperties.max << std::endl;
            if (freqProperties.onSubdevice) {
                std::cout << "freqProperties.subdeviceId = " << freqProperties.subdeviceId << std::endl;
            }
        }

        VALIDATECALL(zesFrequencyGetState(handle, &freqState));
        if (verbose) {
            std::cout << "freqState.currentVoltage = " << freqState.currentVoltage << std::endl;
            std::cout << "freqState.request = " << freqState.request << std::endl;
            std::cout << "freqState.tdp = " << freqState.tdp << std::endl;
            std::cout << "freqState.efficient = " << freqState.efficient << std::endl;
            std::cout << "freqState.actual = " << freqState.actual << std::endl;
            std::cout << "freqState.throttleReasons = " << freqState.throttleReasons << std::endl;
        }

        VALIDATECALL(zesFrequencyGetRange(handle, &freqRange));
        if (verbose) {
            std::cout << "freqRange.min = " << freqRange.min << std::endl;
            std::cout << "freqRange.max = " << freqRange.max << std::endl;
        }
        count = 0;
        VALIDATECALL(zesFrequencyGetAvailableClocks(handle, &count, nullptr));
        std::vector<double> frequency(count);
        VALIDATECALL(zesFrequencyGetAvailableClocks(handle, &count, frequency.data()));
        if (verbose) {
            for (auto freq : frequency) {
                std::cout << " frequency = " << freq << std::endl;
            }
        }
        if (iamroot) {
            // Test setting min and max frequency the same, then restore originals
            testFreqRange.min = freqRange.min;
            testFreqRange.max = freqRange.min;
            if (verbose) {
                std::cout << "Setting Frequency Range . min " << testFreqRange.min << std::endl;
                std::cout << "Setting Frequency Range . max " << testFreqRange.max << std::endl;
            }
            VALIDATECALL(zesFrequencySetRange(handle, &testFreqRange));
            VALIDATECALL(zesFrequencyGetRange(handle, &testFreqRange));
            if (verbose) {
                std::cout << "After Setting Getting Frequency Range . min " << testFreqRange.min << std::endl;
                std::cout << "After Setting Getting Frequency Range . max " << testFreqRange.max << std::endl;
            }
            testFreqRange.min = freqRange.min;
            testFreqRange.max = freqRange.max;
            if (verbose) {
                std::cout << "Setting Frequency Range . min " << testFreqRange.min << std::endl;
                std::cout << "Setting Frequency Range . max " << testFreqRange.max << std::endl;
            }
            VALIDATECALL(zesFrequencySetRange(handle, &testFreqRange));
            VALIDATECALL(zesFrequencyGetRange(handle, &testFreqRange));
            if (verbose) {
                std::cout << "After Setting Getting Frequency Range . min " << testFreqRange.min << std::endl;
                std::cout << "After Setting Getting Frequency Range . max " << testFreqRange.max << std::endl;
            }
        } else {
            std::cout << "Not running as Root. Skipping zetSysmanFrequencySetRange test." << std::endl;
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
    testSysmanFrequency(devices[0]);
}
