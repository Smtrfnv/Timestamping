#include "stream.hpp"

#include <iostream>

#include <linux/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace ts
{

SocketPair createTcpPair()
{
    SocketPair spair = {};
    spair.transport = Transport::TCP;

    const int port = 2552;
    int listenfd = createSocket (AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in servaddr = {};   
    servaddr.sin_family = AF_INET;
    // servaddr.sin_addr.s_addr = htonl ("127.0.0.1");
    servaddr.sin_port = htons (port);

    int res = inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    if(res != 1)
    {
        raiseError("Server: failed to inet_pton");
    }

    spair.serveraddr_in = servaddr;

    res = bind(listenfd, (sockaddr *) &servaddr, sizeof(servaddr));
    if(res != 0)
    {
        raiseError("Server: failed to do bind");
    }

    res = listen(listenfd, 1);
    if(res != 0)
    {
        raiseError("Server: Failed to listen");
    }

    {
        spair.clientFd = createSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        sockaddr_in servaddr = {};
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        int res = inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
        if(res != 1)
        {
            raiseError("Client: failed to inet_pton");
        }
        
        res = connect(spair.clientFd, (sockaddr*) &servaddr, sizeof(servaddr));
        if(res != 0)
        {
            raiseError("Client: failed to connect");
        }

        std::cout << "Client: connectrion established\n";
    }

    
    sockaddr_in cliaddr = {};
    socklen_t clilen = sizeof(clilen);

    spair.serverFd = accept(listenfd, (sockaddr*) &cliaddr, &clilen);
    if(spair.serverFd == -1)
    {
        raiseError("Server: failed to accept connection\n");
    }

    std::cout << "Server: connection accepted\n";
    
    res = close(listenfd);

    return spair;
}

}