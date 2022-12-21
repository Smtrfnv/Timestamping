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
    servaddr.sin_port = htons (port);

    int res = inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    if(res != 1)
    {
        raiseError("createTcpPair: failed to inet_pton");
    }

    spair.serveraddr = servaddr;

    res = bind(listenfd, (sockaddr *) &servaddr, sizeof(servaddr));
    if(res != 0)
    {
        close(listenfd);
        raiseError("createTcpPair: failed to do bind");
    }

    res = listen(listenfd, 1);
    if(res != 0)
    {
        close(listenfd);
        raiseError("createTcpPair: Failed to listen");
    }

    {
        int clientFd = createSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        
        spair.clientFd = std::make_shared<SocketWrapper>("Client_tcp", clientFd, Transport::TCP);

        sockaddr_in servaddr = {};
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        int res = inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
        if(res != 1)
        {
            raiseError("createTcpPair: failed to inet_pton");
        }
        
        res = connect(spair.clientFd->getFd(), (sockaddr*) &servaddr, sizeof(servaddr));
        if(res != 0)
        {
            raiseError("createTcpPair: failed to connect");
        }

        std::cout << "createTcpPair: connectrion established\n";
    }

    
    sockaddr_in cliaddr = {};
    socklen_t clilen = sizeof(clilen);

    int serverFd = accept(listenfd, (sockaddr*) &cliaddr, &clilen);
    if(serverFd == -1)
    {
        raiseError("createTcpPair: failed to accept connection\n");
    }
    spair.serverFd = std::make_shared<SocketWrapper>("Server_tcp", serverFd, Transport::TCP);

    std::cout << "createTcpPair: connection accepted\n";
    
    res = close(listenfd);
    if(res != 0)
    {
        raiseError("createTcpPair: failed to close");
    }

    return spair;
}

SocketPair createStreamLocalPair()
{
    SocketPair spair = {};
    spair.transport = Transport::TCP_LOCAL;

    const char* sockPathServer = "ts.server";
    sockaddr_un serveraddr = {};
    serveraddr.sun_family = AF_UNIX;
    strncpy(serveraddr.sun_path, sockPathServer, sizeof(serveraddr.sun_path));

    spair.serveraddr = serveraddr;

    int listenFd = 0;
    {
        listenFd = createSocket(AF_UNIX, SOCK_STREAM, 0);

        unlink(sockPathServer);

        int res = bind(listenFd, (sockaddr*) &serveraddr, sizeof(serveraddr));
        if(res != 0)
        {
            close(listenFd);
            raiseError("createStreamLocalPair: failed to bind");
        }

        res = listen(listenFd, 1);
        if(res != 0)
        {
            close(listenFd);
            raiseError("createStreamLocalPair: Failed to listen");
        }

    }

    {
        int clientFd = createSocket(AF_UNIX, SOCK_STREAM, 0);

        spair.clientFd = std::make_shared<SocketWrapper>("Client_tcp_local", clientFd, Transport::TCP_LOCAL);

        int res = connect(spair.clientFd->getFd(), (struct sockaddr *) &serveraddr, sizeof(serveraddr));
        if(res != 0)
        {
            close(listenFd);
            raiseError("Client: failed to connect");
        }
    }
    
    {
        sockaddr_un clientAddress = {};
        socklen_t clientAddrLen = sizeof(clientAddress);

        int serverFd = accept(listenFd, (sockaddr*)&clientAddress, &clientAddrLen);
        if(serverFd == -1)
        {
            raiseError("createStreamLocalPair: failed to accept connection\n");
        }
        spair.serverFd = std::make_shared<SocketWrapper>("Server_tcp_local", serverFd, Transport::TCP_LOCAL);
    }

    int res = close(listenFd);
    if(res != 0)
    {
        raiseError("createStreamLocalPair: failed to close");
    }

    return spair;
}

int createStreamSocket()
{
    return createSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);;
}

int createStreamLocalSocket()
{
    return createSocket(AF_UNIX, SOCK_STREAM, 0);
}

}