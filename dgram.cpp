#include "dgram.hpp"

#include "Logger.hpp"

#include <iostream>

#include <linux/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>

namespace ts
{

int createDgramSocket()
{
    return createSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

int createDgramLocalSocket()
{
    return createSocket(AF_UNIX, SOCK_DGRAM, 0);
}

}
