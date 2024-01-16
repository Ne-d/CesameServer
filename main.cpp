#include "CPUMonitor.h"
#include "GPUMonitor.h"
#include "MemoryMonitor.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include <sys/socket.h>
#include <netinet/in.h>

using namespace boost::interprocess;

void cleanup() {
    std::cout << "Removing SHM Objects." << std::endl;
    shared_memory_object::remove("CesameServer");
}

void signal_handler(int signalNum) {
    std::cout << "Received signal " << signalNum << ", cleaning up and exiting." << std::endl;
    cleanup();
    exit(EXIT_SUCCESS);
}

int main()
{
    // Setup signal handling to cleanup before quitting.
    assert(signal(SIGINT, signal_handler) != SIG_ERR);
    assert(signal(SIGQUIT, signal_handler) != SIG_ERR);
    assert(signal(SIGABRT, signal_handler) != SIG_ERR);

    // Create socket file descriptor
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd != -1);

    // FOrcefully attatch socket to port 8080
    int opt = 1;
    int res = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    assert(res != -1);
    std::cout << "setsockopt done" << std::endl;

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    res = bind(sockfd, (struct sockaddr*) &address, sizeof(address));
    std::cout << strerror(errno) << std::endl;
    assert(res != -1);

    res = listen(sockfd, 3);
    assert(res != -1);

    socklen_t addrlen = sizeof(address);
    int new_socket = accept(sockfd, (struct sockaddr*) &address, &addrlen);
    std::cout << "accept() returned " << strerror(errno) << std::endl;
    assert(new_socket != -1);
    std::cout << "accept done" << std::endl;

    std::string str = "Hello there!";
    const char* buffer = str.c_str();
    size_t size = strlen(buffer) * sizeof(char);
    res = send(new_socket, &buffer, size, 0);
    assert(res != -1);
    assert(res != 0);
    assert(res == size);
    std::cout << "send done" << std::endl;

    close(new_socket);
    close(sockfd);

    // TODO: Determine optimized size.
    size_t shmSize = 655360000;

    // Delete previous SHM if it exists.
    cleanup();

    // Create unrestricted managed shared memory object.
    std::cout << "Creating shared memory objects." << std::endl;
    permissions unrestrictedPermissions;
    unrestrictedPermissions.set_unrestricted();
    managed_shared_memory* shm = new managed_shared_memory(create_only, "CesameServer", shmSize, 0, unrestrictedPermissions);
    std::cout << "Running..." << std::endl;
    
    Cesame::Server::CPUMonitor* cpuMon = new Cesame::Server::CPUMonitor(shm);
    Cesame::Server::GPUMonitor* gpuMon = new Cesame::Server::GPUMonitor(shm, 0);
    Cesame::Server::MemoryMonitor* memoryMon = new Cesame::Server::MemoryMonitor(shm);

    // Inifinite loop, the program will use signals to exit.
    while(true) {
        cpuMon->update();
        gpuMon->updateShm();
        memoryMon->update();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    cleanup();

    return EXIT_SUCCESS;
}
