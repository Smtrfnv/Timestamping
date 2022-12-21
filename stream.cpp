#include "stream.hpp"

#include <linux/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace ts
{

int createStreamSocket()
{
    return createSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);;
}

int createStreamLocalSocket()
{
    return createSocket(AF_UNIX, SOCK_STREAM, 0);
}

}