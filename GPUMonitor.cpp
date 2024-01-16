#include "GPUMonitor.h"
#include "common.h"

#include <iostream>
#include <nvml.h>

using namespace Cesame::Server;

GPUMonitor::GPUMonitor(boost::interprocess::managed_shared_memory* inShm, int deviceIndex) {
    shm = inShm;

    nvmlReturn_t nvmlReturn;
    unsigned int deviceCount;

    nvmlReturn = nvmlInit_v2();
    if(nvmlReturn != NVML_SUCCESS) {
        throw new GPUMonitor::NVMLInitException;
    }

    nvmlReturn = nvmlDeviceGetCount_v2(&deviceCount);
    if(nvmlReturn != NVML_SUCCESS) {
        throw new GPUMonitor::NVMLDeviceCountException;
    }

    nvmlReturn = nvmlDeviceGetHandleByIndex_v2(deviceIndex, &device); // Device variable is set to chosen device
    if(nvmlReturn != NVML_SUCCESS) {
        throw new GPUMonitor::NVMLGetHandleException;
    }

    constructShm();
}

unsigned int GPUMonitor::getUsage()
{
    nvmlReturn_t nvmlReturn;
    nvmlUtilization_t* utilization = new nvmlUtilization_t;

    nvmlReturn = nvmlDeviceGetUtilizationRates(device, utilization);

    if(nvmlReturn == NVML_SUCCESS) {
        return utilization->gpu;
    }
    else {
        throw(new UsageQueryException);
    }
}

unsigned int GPUMonitor::getTemperature()
{
    nvmlReturn_t nvmlReturn;
    unsigned int* temp = new unsigned int;

    nvmlReturn = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, temp);

    if(nvmlReturn == NVML_SUCCESS) {
        return *temp;
    }
    else {
        throw(new TemperatureQueryException);
    }
}

unsigned int GPUMonitor::getPower()
{
    nvmlReturn_t nvmlReturn;
    unsigned int* power = new unsigned int;

    nvmlReturn = nvmlDeviceGetPowerUsage(device, power);

    if(nvmlReturn == NVML_SUCCESS) {
        return *power * 1000; // NVML returns a value in milliwatts, we convert it to watts.
    }
    else {
        throw(new PowerQueryException);
    }
}

unsigned int GPUMonitor::getClockSpeed(nvmlClockType_t clockType)
{
    nvmlReturn_t nvmlReturn;
    nvmlClockId_t* clockId;
    unsigned int* clockSpeed = new unsigned int;

    nvmlReturn = nvmlDeviceGetClock(device, clockType, NVML_CLOCK_ID_CURRENT, clockSpeed);

    if(nvmlReturn == NVML_SUCCESS) {
        return *clockSpeed;
    }
    else {
        throw(new ClockSpeedQueryException);
    }

}

unsigned long long GPUMonitor::getVRAMTotal()
{
    nvmlReturn_t nvmlReturn;
    nvmlMemory_t* memory = new nvmlMemory_t;

    nvmlReturn = nvmlDeviceGetMemoryInfo(device, memory);

    if(nvmlReturn == NVML_SUCCESS) {
        return memory->total;
    }
    else {
        throw(new VRAMQueryException);
    }
}

unsigned long long GPUMonitor::getVRAMUsed()
{
    nvmlReturn_t nvmlReturn;
    nvmlMemory_t* memory = new nvmlMemory_t;

    nvmlReturn = nvmlDeviceGetMemoryInfo(device, memory);

    if(nvmlReturn == NVML_SUCCESS) {
        return memory->used;
    }
    else {
        throw(new VRAMQueryException);
    }
}

unsigned long long GPUMonitor::getVRAMFree()
{
    nvmlReturn_t nvmlReturn;
    nvmlMemory_t* memory = new nvmlMemory_t;

    nvmlReturn = nvmlDeviceGetMemoryInfo(device, memory);

    if(nvmlReturn == NVML_SUCCESS) {
        return memory->free;
    }
    else {
        throw(new VRAMQueryException);
    }
}

void GPUMonitor::constructShm()
{
    shmUsage = shm->construct<double>(GPUUsageKey)(getUsage());
    shmTemperature = shm->construct<double>(GPUTemperatureKey)(getTemperature());
    shmPower = shm->construct<double>(GPUPowerKey)(getPower());
    shmClockSpeed = shm->construct<double>(GPUClockSpeedKey)(getClockSpeed(NVML_CLOCK_GRAPHICS));

    shmVRAMTotal = shm->construct<unsigned long long>(GPUVRAMTotalKey)(getVRAMTotal());
    shmVRAMUsed = shm->construct<unsigned long long>(GPUVRAMUsedKey)(getVRAMUsed());
    shmVRAMFree = shm->construct<unsigned long long>(GPUVRAMFreeKey)(getVRAMFree());
}

// This is just a wrapper to make all monitors use the same syntax
void GPUMonitor::update() {
    updateShm();
}

void GPUMonitor::updateShm() {
    *shmUsage = getUsage();
    *shmTemperature = getTemperature();
    *shmPower = getPower();
    *shmClockSpeed = getClockSpeed(NVML_CLOCK_GRAPHICS);

    *shmVRAMTotal = getVRAMTotal();
    *shmVRAMUsed = getVRAMUsed();
    *shmVRAMFree = getVRAMFree();
}
