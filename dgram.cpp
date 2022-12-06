#include "dgram.hpp"

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
        spair.serverFd = createSocket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        sockaddr_in servaddr = {};
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);

        int res = inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
        if(res != 1)
        {
            raiseError("Server: failed to inet_pton");
        }

        spair.serveraddr_in = servaddr;

        res = bind(spair.serverFd, (sockaddr *)&servaddr, sizeof(servaddr));
        if(res != 0)
        {
            raiseError("Server: failed to bind");
        }
    }        

    {
        spair.clientFd = createSocket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);   
    }

    return spair;
}

SocketPair createDgramLocalPair()
{
    SocketPair spair = {};
    spair.transport = Transport::UDP_LOCAL;
    
    const char* sockPathServer = "ts.server";

    {
        spair.serverFd = createSocket (AF_UNIX, SOCK_DGRAM, 0);

        sockaddr_un servaddr = {};
        servaddr.sun_family = AF_UNIX;
        strncpy(servaddr.sun_path, sockPathServer, sizeof(servaddr.sun_path));

        unlink(sockPathServer);

        int res = bind(spair.serverFd, (sockaddr*) &servaddr, sizeof(servaddr));
        if(res  != 0)
        {
            raiseError("Server: failed to bind");
        }

        spair.serveraddr_un = servaddr;
    }

    {
        const char* sockPathClient = "ts.client";

        spair.clientFd = createSocket (AF_UNIX, SOCK_DGRAM, 0);

        sockaddr_un clientaddr = {};
        clientaddr.sun_family = AF_UNIX;
        strncpy(clientaddr.sun_path, sockPathClient, sizeof(clientaddr.sun_path));

        unlink(sockPathClient);

        int res = bind(spair.clientFd, (sockaddr*) &clientaddr, sizeof(clientaddr));
        if(res  != 0)
        {
            raiseError("Server: failed to bind");
        }
        
    }

    return spair;
}

}
