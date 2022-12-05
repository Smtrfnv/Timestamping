#include "dgram.hpp"

#include <iostream>

#include <linux/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace ts
{

SocketPair createUdpPair()
{
    SocketPair spair = {};

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

        spair.serveraddr = servaddr;

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

}
