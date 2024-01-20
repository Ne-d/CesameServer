#include "Network.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <unistd.h>

namespace Cesame::Server::Network {


MonitorPacket* packet = new MonitorPacket;
int sock = -1;
int newSock = 1;


MonitorPacket* getPacket() {
    return packet;
}

static void socketCreate() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock != -1);
}

static void socketAttatch() {
    int opt = 1;
    int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    assert(res != -1);
}

static sockaddr_in createAddress() {
    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    return address;
}

static void socketBind(sockaddr_in address) {
    int res = bind(sock, (struct sockaddr*) &address, sizeof(address));
    assert(res != -1);
}

static void socketListen() {
    int res = listen(sock, 3);
    assert(res != -1);
}

static int socketAttatch(sockaddr_in address) {
    socklen_t addrlen = sizeof(address);
    int newSocket = accept(sock, (struct sockaddr*) &address, &addrlen);
    assert(newSocket != -1);
    return newSocket;
}

void init() {
    socketCreate();
    sockaddr_in address = createAddress();

    socketAttatch();
    socketBind(address);
    socketListen();
    newSock = socketAttatch(address);
}

void socketClose() {
    int res = close(sock);
    assert(res != -1);

    res = close(newSock);
    assert(res != -1);
}

void sendPacket() {
    int res = send(newSock, packet, sizeof(MonitorPacket), 0);
    assert(res != -1);
    assert(res != 0);
    assert(res == sizeof(MonitorPacket));
}


} // namespace Cesame::Server::Network
