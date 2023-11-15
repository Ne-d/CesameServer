#include "CPUMonitor.h"

#include <iostream>
#include <math.h>
#include <sstream>

#include <fcntl.h>
#include <unistd.h>

namespace Cesame::Server {


CPUMonitor::CPUMonitor(boost::interprocess::managed_shared_memory* shm)
{
    // Initialization of timings
    timePointCurrent = std::chrono::steady_clock::now();

    // Initialization of file streams
    statStream.open(statFile);
    if(!statStream.is_open()) {
        std::cerr << "ERROR in cpuMonitor::cpuMonitor(): could not open /proc/stat." << std::endl;
    }

    tempStream.open(tempFile);
    if(!tempStream.is_open()) {
        std::cerr << "ERROR in cpuMonitor::cpuMonitor(): could not open temperature file." << std::endl;
    }

    infoStream.open(infoFile);
    if(!infoStream.is_open()) {
        std::cerr << "ERROR in cpuMonitor::cpuMonitor(): could not open /proc/cpuinfo." << std::endl;
    }

    // Preparation of data vectors (arrays).
    coreCount = 16; // TODO: Read the "siblings" value from infoFile to adapt to any CPU, or smth else idk man

    fields.resize(coreCount + 1);
    totalTime.resize(coreCount + 1, 0);
    prevTotalTime.resize(coreCount + 1, 0);
    activeTime.resize(coreCount + 1, 0);
    prevActiveTime.resize(coreCount + 1, 0);

    usagePerCore.resize(coreCount, 0);
    temperaturePerCore.resize(coreCount, 0);
    powerPerCore.resize(coreCount, 0);
    clockSpeedPerCore.resize(coreCount, 0);

    for(unsigned int i = 0; i <= coreCount; i++) {
        fields.at(i).resize(10, 0);
    }

    // Preparation of rapl access
    detectPackages();

    coreEnergy.resize(totalCores / 2);
    coreEnergyPrevious.resize(totalCores/ 2);
    packageEnergy.resize(totalPackages);
    packageEnergyPrevious.resize(totalPackages);

    coreEnergyUnits = readMsr(0, AMD_MSR_PWR_UNIT);

    timeUnit = (coreEnergyUnits & AMD_TIME_UNIT_MASK) >> 16;
    energyUnit = (coreEnergyUnits & AMD_ENERGY_UNIT_MASK) >> 8;
    powerUnit = (coreEnergyUnits & AMD_POWER_UNIT_MASK);

    timeUnitAdjusted = pow(0.5, (double)(timeUnit));
    energyUnitAdjusted = pow(0.5, (double)(energyUnit));
    powerUnitAdjusted = pow(0.5, (double)(powerUnit));

    constructShm(shm);

    update();
}

void CPUMonitor::updateUsage() {
    std::string line;
    std::stringstream iss;
    std::string field;

    statStream.seekg(0, statStream.beg); // Seek to the begining of the file

    for(unsigned int l = 0; l < coreCount; l++) { // For every CPU line
        //if(l == 0) { continue; } // Ignore the first line that counts the total/average of all cores.
        int fieldNb = 0;
        getline(statStream, line);
        iss.clear();
        iss.str(line);

        while(getline(iss, field, ' ')) { // For every field in the current line
            if(field == "" || field.at(0) == 'c') { continue; } // Ignore the first token indicating cpu number (cpu0, cpu1, cpu2, etc.)

            fields.at(l).at(fieldNb) = stoi(field); // Write the data from the field into the corresponding variable
            fieldNb++;
        }
        iss.str(std::string()); // Reset the string in the stream
    }

    prevTotalTime = totalTime;
    prevActiveTime = activeTime;

    // Reset values of totalTime and activeTime to allow for the next loop to increment them
    for(unsigned int i = 0; i < coreCount; i++) {
        totalTime.at(i) = 0;
        activeTime.at(i) = 0;
    }

    for(unsigned int l = 0; l < coreCount; l++) { // Loop to calculate totalTime and activeTime
        for(unsigned int f = 0; f <= 9; f++) { // For every field
            totalTime.at(l) += fields.at(l).at(f); // Cumulate values of the fields into totalTime

            if(f != 3 && f != 4) { // Cumulate values into activeTime only if they are not idle or iowait (fields 3 and 4)
                activeTime.at(l) += fields.at(l).at(f);
            }
        }
    }

    // Update variables to store all monitoring data
    for(unsigned int i = 0; i < coreCount; i++) {
        usagePerCore.at(i) = (((double)activeTime.at(i) - (double)prevActiveTime.at(i)) /
                              ((double)totalTime.at(i) - (double)prevTotalTime.at(i))) * 100.0;
    }
}

void CPUMonitor::updateTemperature() {
    tempStream.seekg(0, tempStream.beg);
    std::string tempBuffer;
    getline(tempStream, tempBuffer);
    temperaturePackage = std::stod(tempBuffer) / 1000;
}

void CPUMonitor::updatePower() {
    updateEnergy();
    deltaTime = timePointCurrent - timePointPrevious;

    for(int i = 0; i < totalPackages; i++) {
        powerPackage = (packageEnergy.at(i) - packageEnergyPrevious.at(i)) / deltaTime.count();
    }

    for(int i = 0; i < totalCores; i++) {
        powerPerCore.at(i) = (coreEnergy.at(i/2) - coreEnergyPrevious.at(i/2)) / deltaTime.count();
    }
}

void CPUMonitor::updateClockSpeeds() {
    std::string line;

    int lineNb = 0;
    double currentCoreClockSpeed = 0;
    double totalClockSpeed = 0;
    std::string clockString = "";

    infoStream.clear();
    infoStream.seekg(0, infoStream.beg);

    while(getline(infoStream, line)) { // For each line of the info file
        if(line.find("cpu MHz") != std::string::npos) {
            line.erase(0, 11);
            currentCoreClockSpeed = stod(line);
            clockSpeedPerCore.at(lineNb) = currentCoreClockSpeed;
            totalClockSpeed += currentCoreClockSpeed;
            lineNb++;
        }
    }

    clockSpeedAverage = totalClockSpeed / (double) coreCount;
}

void CPUMonitor::update() {
    updateUsage();
    updateTemperature();
    updatePower();
    updateClockSpeeds();

    updateShm();
}

void CPUMonitor::detectPackages()
{
    char filename[BUFSIZ];
    FILE *fff;
    int package;
    unsigned int i = 0;

    // Initialize all values of packageMap to -1
    for(i = 0; i < MAX_PACKAGES; i++)
    {
        packageMap.push_back(-1);
    }

    for(i = 0; i < MAX_CPUS; i++) // For all cpu cores
    {
        // Set filename to the current cpu core's physical_package_id file.
        sprintf(filename, "/sys/devices/system/cpu/cpu%d/topology/physical_package_id", i);

        // Open the file, to read its content (number) and write it to package.
        fff = fopen(filename, "r");
        if(fff == NULL) break;

        fscanf(fff, "%d", &package);
        fclose(fff);

        if(packageMap.at(package) == -1)
        {
            totalPackages++;
            packageMap.at(package) = i;
        }
    }

    totalCores = i;
}

long long CPUMonitor::readMsr(int core, unsigned int reg)
{
    uint64_t data;
    int fd;
    char msrFilename[255];

    sprintf(msrFilename, "/dev/cpu/%d/msr", core);

    fd = open(msrFilename, O_RDONLY);

    if (fd < 0)
    {
        if (errno == ENXIO)
        {
            std::cerr <<"readMsr: No CPU" << core << std::endl;
            throw new CPUMonitor::MSRNoCpuException;
        }
        else if (errno == EIO)
        {
            std::cerr << "readMsr: CPU " << core << " doesn't support MSRs" << std::endl;
            throw new CPUMonitor::MSRUnsupportedCpuException;
        }
        else
        {
            perror("readMsr: open");
            std::cerr << "This program needs to be run as root to access the MSR." << std::endl;
            throw new CPUMonitor::MSROpenException;
        }
    }

    if (pread(fd, &data, sizeof data, reg) != sizeof data)
    {
        if (errno == EIO)
        {
            std::cerr << "readMsr: CPU " << core << " cannot read MSR " << reg << std::endl;
            throw new CPUMonitor::MSRFailedReadException;
        } else {
            perror("readMsr: pread");
            throw new CPUMonitor::MSRUnknownReadException;
        }
    }

    close(fd);

    return (long long) data;
}

void CPUMonitor::updateEnergy()
{
    timePointPrevious = timePointCurrent;
    timePointCurrent = std::chrono::steady_clock::now();

    for(int i = 0; i < totalCores / 2; i++)
    {
        coreEnergyPrevious.at(i) = coreEnergy.at(i);
        int coreEnergyRaw = readMsr(i, AMD_MSR_CORE_ENERGY);
        coreEnergy.at(i) = coreEnergyRaw * energyUnitAdjusted;
    }

    for(int i = 0; i < totalPackages; i++)
    {
        packageEnergyPrevious.at(i) = packageEnergy.at(i);
        int packageEnergyRaw = readMsr(i, AMD_MSR_PACKAGE_ENERGY);
        packageEnergy.at(i) = packageEnergyRaw * energyUnitAdjusted;
    }
}

using namespace boost::interprocess;
void CPUMonitor::constructShm(boost::interprocess::managed_shared_memory* shm) {
    shmUsagePerCore = shm->construct<std::vector<double>>("CPUUsagePerCore")(usagePerCore);
    shmCoreCount = shm->construct<unsigned int>("CPUCoreCount")(coreCount);

    shmTemperaturePerCore = shm->construct<std::vector<double>>("CPUTemperaturePerCore")(temperaturePerCore);
    shmTemperaturePackage = shm->construct<double>("CPUTemperaturePackage")(temperaturePackage);

    shmPowerPerCore = shm->construct<std::vector<double>>("CPUPowerPerCore")(powerPerCore);
    shmPowerPackage = shm->construct<double>("CPUPowerPackage")(powerPackage);

    shmClockSpeedPerCore = shm->construct<std::vector<double>>("CPUClockSpeedPerCore")(clockSpeedPerCore);
    shmClockSpeedAverage = shm->construct<double>("CPUClockSpeedAverage")(clockSpeedAverage);
}

void CPUMonitor::updateShm()
{
    *shmUsagePerCore = usagePerCore;
    // CoreCount should not change at runtime after its initialization.
    // Cause I don't think you're gonna be hotswapping your CPU mate.

    *shmTemperaturePerCore = temperaturePerCore;
    *shmTemperaturePackage = temperaturePackage;

    *shmPowerPerCore = powerPerCore;
    *shmPowerPackage = powerPackage;

    *shmClockSpeedPerCore = clockSpeedPerCore;
    *shmClockSpeedAverage = clockSpeedAverage;
}

} // namespace Cesame::Server
