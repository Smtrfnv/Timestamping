#pragma once

#include "Transport.hpp"
#include "SocketOptions.hpp"

#include <optional>
#include <string>

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

namespace ts
{

int createSocket(int family, int type, int protocol);
// create socket and set options. Return -1 in case of a failure

bool setOptions(int fd, int family, const SocketOptions& opts);

std::optional<sockaddr_in> getIpV4AddressAndPort(const std::string& addrAndPort);
sockaddr_un convertUnixSocketAddr(const std::string& addr);



}

