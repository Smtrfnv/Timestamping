#include "util.hpp"

#include "stream.hpp"
#include "dgram.hpp"
#include "Logger.hpp"

#include <iostream>
#include <cstring>
#include <cstdarg>
#include <vector>

#include <sys/socket.h>
#include <linux/net_tstamp.h>
#include <unistd.h>
#include <arpa/inet.h>

namespace ts
{

void raiseError(const char* format, ...)
{
    const size_t CAPACITY = 1024;
    char buf[CAPACITY];

    va_list args;
    va_start(args, format);

    vsnprintf(buf, CAPACITY, format, args);

    va_end(args);

    std::cerr << buf << "\n last error = " << strerror(errno) << std::endl;
    exit(1);
}

int createSocket(int family, int type, int protocol)
{
    TSLOG("creating socket");

    int fd = socket(family, type, protocol);
    if(fd < 0)
    {
        TSLOG("socket creation error");
        return -1;
    }

    
    uint32_t val = 1;

    if(family == AF_INET)
    {
        val =  
            // SOF_TIMESTAMPING_TX_HARDWARE
              SOF_TIMESTAMPING_TX_SOFTWARE |

            //  SOF_TIMESTAMPING_RX_SOFTWARE |
            // | SOF_TIMESTAMPING_RX_HARDWARE |

            SOF_TIMESTAMPING_SOFTWARE 
            // | SOF_TIMESTAMPING_RAW_HARDWARE
            ;
    }
    
    int res = setsockopt(fd, SOL_SOCKET, (family == AF_INET) ? SO_TIMESTAMPING : SO_TIMESTAMPNS, &val, sizeof(val));

    if(res != 0)
    {
        TSLOG("Failed to setsockopt");
        close(fd);
        return -1;
    }

    int v = 1;
    res = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(v));
    if(res != 0)
    {
        raiseError("Failed to setsockopt");
        close(fd);
        return -1;
    }

    return fd;
}



std::optional<sockaddr_in> getIpV4AddressAndPort(const std::string& addrAndPort)
{
    const char delimeter = ':';

    int pos = addrAndPort.find(delimeter, 0);
    if(pos == std::string::npos)
    {
        return std::nullopt;
    }

    const std::string addr = addrAndPort.substr(0, pos);
    const std::string port = addrAndPort.substr(pos + 1);

    if(addr.empty() || port.empty())
    {
        return std::nullopt;
    }

    sockaddr_in saddr = {};
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(std::stoul(port));

    if(inet_pton(AF_INET, addr.c_str(), &saddr.sin_addr) != 1)
    {
        return std::nullopt;
    }

    return saddr;

}

sockaddr_un convertUnixSocketAddr(const std::string& addr)
{
    sockaddr_un res = {};
    res.sun_family = AF_UNIX;
    strncpy(res.sun_path, addr.c_str(), sizeof(res.sun_path));
    return res;
}


}