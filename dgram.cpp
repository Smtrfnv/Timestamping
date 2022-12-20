#include "dgram.hpp"

#include "Logger.hpp"

#include <iostream>

#include <linux/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>

namespace ts
{

SocketPair createUdpPair()
{
    SocketPair spair = {};
    spair.transport = Transport::UDP;

    const int port = 2552;

    {
        int serverFd = createSocket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        spair.serverFd = std::make_shared<SocketWrapper>("Server_udp", serverFd, Transport::UDP);

        sockaddr_in servaddr = {};
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);

        int res = inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
        if(res != 1)
        {
            raiseError("Server_udp: failed to inet_pton");
        }
        TSLOG("addr %u, port %u", servaddr.sin_addr.s_addr, servaddr.sin_port);

        spair.serveraddr = servaddr;

        res = bind(spair.serverFd->getFd(), (sockaddr *)&servaddr, sizeof(servaddr));
        if(res != 0)
        {
            raiseError("Server_udp: failed to bind");
        }
    }        

    
    int clientFd = createSocket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    spair.clientFd = std::make_shared<SocketWrapper>("Client_udp", clientFd, Transport::UDP);

    return spair;
}

SocketPair createDgramLocalPair()
{
    SocketPair spair = {};
    spair.transport = Transport::UDP_LOCAL;
    
    const char* sockPathServer = "ts.server";

    {
        int serverFd = createSocket (AF_UNIX, SOCK_DGRAM, 0);

        spair.serverFd = std::make_shared<SocketWrapper>("Server_udp_local", serverFd, Transport::UDP_LOCAL);

        sockaddr_un servaddr = {};
        servaddr.sun_family = AF_UNIX;
        strncpy(servaddr.sun_path, sockPathServer, sizeof(servaddr.sun_path));

        unlink(sockPathServer);

        int res = bind(spair.serverFd->getFd(), (sockaddr*) &servaddr, sizeof(servaddr));
        if(res  != 0)
        {
            raiseError("Server_udp_local: failed to bind");
        }

        spair.serveraddr = servaddr;
    }

    {
        const char* sockPathClient = "ts.client";

        int clientFd = createSocket (AF_UNIX, SOCK_DGRAM, 0);
        spair.clientFd = std::make_shared<SocketWrapper>("Client_udp_local", clientFd, Transport::UDP_LOCAL);

        sockaddr_un clientaddr = {};
        clientaddr.sun_family = AF_UNIX;
        strncpy(clientaddr.sun_path, sockPathClient, sizeof(clientaddr.sun_path));

        unlink(sockPathClient);

        int res = bind(spair.clientFd->getFd(), (sockaddr*) &clientaddr, sizeof(clientaddr));
        if(res  != 0)
        {
            raiseError("Client_udp_local: failed to bind");
        }
        
    }

    return spair;
}

int createDgramSocket()
{
    int serverFd = createSocket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    return serverFd;
}

}
