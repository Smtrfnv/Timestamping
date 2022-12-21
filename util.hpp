#pragma once

#include "Transport.hpp"
#include "SocketPair.hpp"

#include <optional>

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

namespace ts
{

void raiseError(const char* format, ...);

SocketPair createSocketPair(Transport t);
void closeSockPair(const SocketPair& p);

int createSocket(int family, int type, int protocol);

std::optional<sockaddr_in> getIpV4AddressAndPort(const std::string& addrAndPort);
sockaddr_un convertUnixSocketAddr(const std::string& addr);
// addrAndPort is a string of format "127.0.0.1:8080"

}

