#pragma once


#include <sys/socket.h>
#include <netinet/in.h>

namespace ts
{

struct SocketPair
{
    int clientFd;
    int serverFd;
    sockaddr_in serveraddr;
};

enum class Transport
{
    TCP,
    UDP
};

void raiseError( const char* x);

int createSocket(int family, int type, int protocol);

void processCtrlMessage(cmsghdr &chdr);

}

