#pragma once

#include "Transport.hpp"
#include "SocketPair.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

namespace ts
{

void raiseError( const char* x);

SocketPair createSocketPair(Transport t);
void closeSockPair(const SocketPair& p);

int createSocket(int family, int type, int protocol);

}

