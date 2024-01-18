#ifndef COMMON_H
#define COMMON_H

#include <vector>

#define SharedMemoryName "CesameServer"

#define CPUUsageAverageKey "CPUUsageAverage"
#define CPUUsageCoreKey "CPUUsagePerCore"
#define CPUTemperaturePackagekey "CPUTemperaturePackage"
#define CPUTemperaturePerCoreKey "CPUTemperaturePerCore"
#define CPUPowerPackageKey "CPUPowerPackage"
#define CPUPowerPerCoreKey "CPUPowerPerCore"
#define CPUClockSpeedAverageKey "CPUClockSpeedAverage"
#define CPUClockSpeedCoreKey "CPUClockSpeedCore"
#define CPUCoreCountKey "CPUCoreCount"

#define GPUUsageKey "GPUUsage"
#define GPUTemperatureKey "GPUTemperature"
#define GPUPowerKey "GPUPower"
#define GPUClockSpeedKey "GPUClockSpeed"
#define GPUVRAMTotalKey "GPUVRAMTotal"
#define GPUVRAMUsedKey "GPUVRAMUsed"
#define GPUVRAMFreeKey "GPUVRAMFree"

#define MemoryTotalKey "MemoryTotal"
#define MemoryUsedKey "MemoryUsed"
#define MemoryFreeKey "MemoryFree"
#define MemoryAvailableKey "MemoryAvailable"

typedef struct packet_t {
    // CPU
    double              CPUUsageAverage;
    std::vector<double> CPUUsagePerCore;

    double              CPUTemperaturePackage;
    std::vector<double> CPUTemperaturePerCore;

    double              CPUPowerPackage;
    std::vector<double> CPUPowerPerCore;

    double              CPUClockSpeedAverage;
    std::vector<double> CPUClockSpeedPerCore;

    unsigned short      CPUCoreCount;


    // GPU
    unsigned int        GPUUsage;
    unsigned int        GPUTemperature;
    unsigned int        GPUPower;
    unsigned int        GPUClockSpeed;
    unsigned long long  GPUVRAMTotal;
    unsigned long long  GPUVRAMUsed;
    unsigned long long  GPUVRAMFree;


    // Memory
    unsigned int        MemoryTotal;
    unsigned int        MemoryUsed;
    unsigned int        MemoryFree;
    unsigned int        MemoryAvailable;
} packet;


#endif // COMMON_H
