#ifndef GPUMONITOR_H
#define GPUMONITOR_H

#include <boost/interprocess/managed_shared_memory.hpp>
#include <string>
#include <nvml.h>

namespace Cesame::Server
{

class GPUMonitor
{
private:
    // NVML objects
    nvmlDevice_t device;

    double VRAMDivisionFactor = 1000*1000*1000;

public: // Methods
    GPUMonitor(int deviceIndex);

    void constructShm(boost::interprocess::managed_shared_memory* shm);
    void updateShm();

    // Main queries

    // Unit: Percent
    unsigned int getUsage();

    // Unit: Degrees Celcius
    unsigned int getTemperature();

    // Unit: Milliwatts
    unsigned int getPower();

    // Unit: Megahertz
    unsigned int getClockSpeed(nvmlClockType_t clockType);

    // Unit: Bytes
    unsigned long long getVRAMTotal();
    unsigned long long getVRAMUsed();
    unsigned long long getVRAMFree();


    // Additionnal queries (if I spend the time to implement them) (holy shit that's a lot of functions)
    /*
    nvmlComputeMode_t getComputeMode();
    nvmlProcessInfo_t getComputeProcesses();
    nvmlProcessInfo_t getGraphicsProcesses();
    unsigned int getPCIeGen();
    unsigned int getPCIeWidth();
    unsigned int getPcieThroughput();
    unsigned int getMaxPCIeGen();
    unsigned int getMaxPCIeWidth();
    unsigned int getMaxPcieSpeed();
    unsigned long long getThrottleReason();
    double getEncoderUsage();
    double getEncoderCapacity();
    void getEncoderStats(); // Still need to define a struct for the return type, void is a placeholder
    double getDecoderUsage();
    double getDriverModel();
    double getEnforcedPowerLimit();
    double getFanSpeed();
    double getMaxClockSpeed();
    double getMaxBoostClockSpeed();
    double getMemoryBusWidth();
    std::string getName();
    int getNumberFans();
    int getNumberCores();
    nvmlPstates_t getPerformanceState();
    double getPowerLimit();
    double getTemperatureThreshold();
    double getTotalEnergyConsumption();
    double getViolationStatus();
    */

public: // Monitoring variables
    // Unit: percent
    unsigned int usage = 0;

    // Unite: Degrees Celsius
    unsigned int temperature = 0;

    // Unit: Milliwatts
    unsigned int power = 0;

    // Unit: Megahertz
    unsigned int clockSpeed = -1;

    // Unit: Bytes
    double VRAMTotal = -1;
    double VRAMUsed = -1;
    double VRAMFree = -1;

private: // Shared Memory Pointers
    unsigned int* shmUsage;
    unsigned int* shmTemperature;
    unsigned int* shmPower;
    unsigned int* shmClockSpeed;

    unsigned long long* shmVRAMTotal;
    unsigned long long* shmVRAMUsed;
    unsigned long long* shmVRAMFree;


public: // Exceptions
    class NVMLInitException : public std::exception {
        const char* what() const noexcept { return "NVML failed to initialize."; };
    };

    class NVMLDeviceCountException : public std::exception {
        const char* what() const noexcept { return "NVML failed to get device count."; };
    };

    class NVMLGetHandleException : public std::exception {
        const char* what() const noexcept { return "NVML failed to get device handle."; };
    };

    class UsageQueryException : public std::exception {
        const char* what() const noexcept { return "NVML failed to query GPU Usage."; };
    };

    class TemperatureQueryException : public std::exception {
        const char* what() const noexcept { return "NVML failed to query GPU Temperature."; };
    };

    class PowerQueryException : public std::exception {
        const char* what() const noexcept { return "NVML failed to query GPU Power."; };
    };

    class ClockSpeedQueryException : public std::exception {
        const char* what() const noexcept { return "NVML failed to query GPU Clock Speed."; };
    };

    class VRAMQueryException : public std::exception {
        const char* what() const noexcept { return "NVML failed to query GPU VRAM Usage."; };
    };
};


}
#endif // GPUMONITOR_H
