#include "GPUMonitor.h"
#include "Network.h"

#include <iostream>
#include <nvml.h>

using namespace Cesame::Server;

GPUMonitor::GPUMonitor(int deviceIndex) {
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

// This is just a wrapper to make all monitors use the same syntax
void GPUMonitor::update() {
    updatePacket();
}

void GPUMonitor::updatePacket() {
    Network::getPacket()->GPUUsage = getUsage();
    Network::getPacket()->GPUTemperature = getTemperature();
    Network::getPacket()->GPUPower = getPower();
    Network::getPacket()->GPUClockSpeed = getClockSpeed(NVML_CLOCK_GRAPHICS);
    Network::getPacket()->GPUVRAMTotal = getVRAMTotal();
    Network::getPacket()->GPUVRAMUsed = getVRAMUsed();
    Network::getPacket()->GPUVRAMFree = getVRAMFree();
}
