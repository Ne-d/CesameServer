#include "CPUMonitor.h"

#include <iostream>
#include <thread>
#include <chrono>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

using namespace boost::interprocess;

int main()
{
    // Delete previous SHM if it exists.
    std::cout << "Deleting previous interprocess objects" << std::endl;
    shared_memory_object::remove("SharedMemory");
    named_mutex::remove("CesameServer");
    std::cout << "Finished deleting previous interprocess objects" << std::endl;

    // Create unrestricted managed shared memory object.
    std::cout << "Creating shared memory objects." << std::endl;
    permissions unrestrictedPermissions;
    unrestrictedPermissions.set_unrestricted();
    managed_shared_memory* managedShm = new managed_shared_memory(create_only, "SharedMemory", 65536, 0, unrestrictedPermissions);
    std::cout << "Finished creating shared memory objects." << std::endl;
    
    Cesame::Server::CPUMonitor *cpuMon = new Cesame::Server::CPUMonitor(managedShm);

    for(int i = 0; i <= 100; i++)
    {
        cpuMon->update();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Removing SHM Objects." << std::endl;
    shared_memory_object::remove("SharedMemory");
    std::cout << "Finished removing SHM Objects." << std::endl;

    return 0;
}
