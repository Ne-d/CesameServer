#ifndef CESAME_SERVER_NETWORK_H
#define CESAME_SERVER_NETWORK_H

#include "common.h"

namespace Cesame {
namespace Server {
namespace Network {

MonitorPacket* getPacket();
void init();
void socketClose();
void sendPacket();

} // namespace Network
} // namespace Server
} // namespace Cesame

#endif // CESAME_SERVER_NETWORK_H
