#pragma once

#include "Transport.hpp"
#include "SocketWrapper.hpp"

#include <memory>
#include <variant>

#include <netinet/in.h>
#include <sys/un.h>

namespace ts
{


struct SocketPair
{
    Transport transport;

    std::shared_ptr<SocketWrapper> clientFd;
    std::shared_ptr<SocketWrapper> serverFd;

    std::variant<sockaddr_in, sockaddr_un> serveraddr;
};


}