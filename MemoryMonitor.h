#ifndef MEMORYMONITOR_H
#define MEMORYMONITOR_H

#include <fstream>
#include <boost/interprocess/managed_shared_memory.hpp>

namespace Cesame::Server
{

class MemoryMonitor {
private: // Member Variables
    // File streams
    std::ifstream infoStream;
    const std::string infoFile = "/proc/meminfo";

    // Monitoring variables (in bytes)
    unsigned int total;
    unsigned int free;
    unsigned int available;
    unsigned int used;

    // Others
    unsigned int conversionFactor = 1000;

public: // Methods
    MemoryMonitor();

    void update();
    void updatePacket();

    std::string trimLine(std::string line);

public: // Exceptions
    class MemoryInfoFileException : public std::exception {
        const char* what() const noexcept { return "MemoryMonitor: failed to open /proc/meminfo"; };
    };
};

} // namespace Cesame::Server

#endif // MEMORYMONITOR_H
