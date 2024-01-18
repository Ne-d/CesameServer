#ifndef CESAME_SERVER_CPUMONITOR_H
#define CESAME_SERVER_CPUMONITOR_H

#include <fstream>
#include <vector>
#include <chrono>

#include <boost/interprocess/managed_shared_memory.hpp>

#define AMD_MSR_PWR_UNIT 0xC0010299
#define AMD_MSR_CORE_ENERGY 0xC001029A
#define AMD_MSR_PACKAGE_ENERGY 0xC001029B

#define AMD_TIME_UNIT_MASK 0xF0000
#define AMD_ENERGY_UNIT_MASK 0x1F00
#define AMD_POWER_UNIT_MASK 0xF
#define STRING_BUFFER 1024


namespace Cesame::Server {

class CPUMonitor {
public: // Methods
    CPUMonitor();
    void update();

private: // Variables to store monitoring data
    std::vector<double> usagePerCore;
    double usageAverage;
    unsigned int coreCount;

    std::vector<double> temperaturePerCore;
    double temperaturePackage = 0;

    std::vector<double> powerPerCore;
    double powerPackage = 0;

    std::vector<double> clockSpeedPerCore;
    double clockSpeedAverage;


private: // Methods
    void updateUsage();
    void updateTemperature();
    void updatePower();
    void updateClockSpeeds();

    void updatePacket();

private:
    // Timings
    std::chrono::time_point<std::chrono::steady_clock> timePointCurrent;
    std::chrono::time_point<std::chrono::steady_clock> timePointPrevious;
    std::chrono::duration<double> deltaTime;


    // File streams
    const std::string statFile = "/proc/stat";
    const std::string infoFile = "/proc/cpuinfo";
    const std::string tempFile = "/sys/class/hwmon/hwmon4/temp1_input"; // TODO: determine automatically / user input

    std::ifstream statStream;
    std::ifstream infoStream;
    std::ifstream tempStream;


    // Raw Data from /proc/stat
    std::vector<std::vector<int>> fields;

    std::vector<int> totalTime;
    std::vector<int> prevTotalTime;

    std::vector<int> activeTime;
    std::vector<int> prevActiveTime;


    // Power Draw
    double energy = 0;
    double prevEnergy = 0;


private: // MSR / RAPL
    const unsigned int MAX_CPUS = 1024;
    const unsigned int MAX_PACKAGES = 16;
    int totalCores = 0;
    int totalPackages = 0;
    std::vector<int> packageMap;
    int coreEnergyUnits;

    unsigned int timeUnit;
    unsigned int energyUnit;
    unsigned int powerUnit;
    double timeUnitAdjusted;
    double energyUnitAdjusted;
    double powerUnitAdjusted;

    std::vector<double> coreEnergy;
    std::vector<double> coreEnergyPrevious;

    std::vector<double> packageEnergy;
    std::vector<double> packageEnergyPrevious;

    void detectPackages();
    long long readMsr(int core, unsigned int reg);
    void updateEnergy();

public: // Exceptions
    class MSRNoCpuException : public std::exception {
        const char* what() const noexcept { return "Failed to read MSR: No CPU."; };
    };


    class MSRUnsupportedCpuException : public std::exception {
        const char* what() const noexcept { return "Failed to read MSR: CPU not supported."; };
    };


    class MSROpenException : public std::exception {
        const char* what() const noexcept { return "Failed to read MSR: Unknown open exception."; };
    };


    class MSRFailedReadException : public std::exception {
        const char* what() const noexcept { return "Failed to read MSR: Reading error."; };
    };


    class MSRUnknownReadException : public std::exception {
        const char* what() const noexcept { return "Failed to read MSR: Unknown read exception."; };
    };
}; // class CPUMonitor

} // namespace Cesame::Server

#endif // CESAME_SERVER_CPUMONITOR_H
