#include "CPUMonitor.h"
#include "GPUMonitor.h"
#include "MemoryMonitor.h"

#include <iostream>
#include <thread>
#include <chrono>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include "Network.h"

using namespace Cesame::Server;

int main() {
    Network::init();

    CPUMonitor* cpuMon = new Cesame::Server::CPUMonitor();
    GPUMonitor* gpuMon = new Cesame::Server::GPUMonitor(0);
    MemoryMonitor* memoryMon = new Cesame::Server::MemoryMonitor();

    // Inifinite loop, the program will use signals to exit.
    while(true) {
        cpuMon->update();
        gpuMon->update();
        memoryMon->update();
        Network::sendPacket();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }


    Network::socketClose();

    return EXIT_SUCCESS;
}
