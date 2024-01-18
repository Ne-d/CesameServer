#include "Network.h"

namespace Cesame {
namespace Server {
namespace Network {

MonitorPacket* packet = new MonitorPacket;

MonitorPacket* getPacket() {
    return packet;
}

} // namespace Network
} // namespace Server
} // namespace Cesame
