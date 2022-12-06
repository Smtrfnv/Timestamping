#pragma once


#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

namespace ts
{

enum class Transport
{
    TCP,
    UDP,
    UDP_LOCAL,
    TCP_LOCAL
};

struct SocketPair
{
    Transport transport;
    int clientFd;
    int serverFd;
    sockaddr_in serveraddr_in; // valid for TCP & UDP
    sockaddr_un serveraddr_un; // valid only for UNIX sockets
};

void raiseError( const char* x);

int createSocket(int family, int type, int protocol);

void processCtrlMessage(cmsghdr &chdr);

}

