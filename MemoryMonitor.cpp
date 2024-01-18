#include "MemoryMonitor.h"
#include "Network.h"

#include <iostream>

using namespace Cesame::Server;

MemoryMonitor::MemoryMonitor() {
    // Initialize file streams
    infoStream.open(infoFile);

    if(!infoStream.is_open()) {
        std::cerr << "ERROR in memoryMonitor::memoryMonitor(): could not open /proc/meminfo." << std::endl;
    }
}


void MemoryMonitor::update() {
    std::string line = "";
    infoStream.seekg(0);

    // Get Total Memory information
    getline(infoStream, line);
    line = trimLine(line);
    total = stoi(line) * conversionFactor;

    // Get Free Memory information
    getline(infoStream, line);
    line = trimLine(line);
    free = stoi(line) * conversionFactor;

    // Get Available Memory information
    getline(infoStream, line);
    line = trimLine(line);
    available = stoi(line) * conversionFactor;

    // Calculate used memory
    used = total - available;

    updatePacket();
}


void MemoryMonitor::updatePacket() {
    Network::getPacket()->MemoryTotal = total;
    Network::getPacket()->MemoryFree = free;
    Network::getPacket()->MemoryAvailable = available;
    Network::getPacket()->MemoryUsed = used;
}


// Absolute hellscape of a function to only keep the numbers in a line from /proc/meminfo
// TODO: Do something.
std::string MemoryMonitor::trimLine(std::string line)
{
    // Removing the last three characters.
    for(int i = 0; i < 3; i++) {
        line.pop_back();
    }

    // IDK how to regex go brrrrrrrrrr
    while(line.front() != '0' && line.front() != '1' && line.front() != '2' && line.front() != '3' && line.front() != '4' &&
           line.front() != '5' && line.front() != '6' && line.front() != '7' && line.front() != '8' && line.front() != '9') {
        line.erase(0, 1);
    }
    // Please kill me.

    return line;
}

