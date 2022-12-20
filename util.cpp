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

SocketPair createSocketPair(Transport t)
{
    SocketPair p;
    if(t == Transport::TCP)
    {
        p = createTcpPair();
        TSLOG("TCP socket pair created");
    }
    else if(t == Transport::TCP_LOCAL)
    {
        p = createStreamLocalPair();
        TSLOG("TCP_LOCAL socket pair created");
    }
    else if(t == Transport::UDP)
    {
        p = createUdpPair();
        TSLOG("UDP socket pair created");
    }
    else if(t == Transport::UDP_LOCAL)
    {
        p = createDgramLocalPair();
        TSLOG("UDP_LOCAL socket pair created");
    }
    else
        raiseError("createSocketPair:: unsupported transport type");
    return p;
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

void closeSockPair(const SocketPair& p)
{
    if(p.transport == Transport::UDP_LOCAL || p.transport == Transport::TCP_LOCAL)
    {
        // unlink(p.);
        // unlink(p.serverFd->getFd());
    }

    if(p.transport == Transport::TCP)
    {
        if(shutdown(p.clientFd->getFd(), SHUT_RDWR) != 0)
        {
            std::cerr << "Failed to shutdown client socket\n";
        }
        
        if(shutdown(p.serverFd->getFd(), SHUT_RDWR) != 0)
        {
            std::cerr << "Failed to shutdown server socket\n";
        }
    }

    int rc = close(p.clientFd->getFd());
    if(rc != 0)
    {
        std::cerr << "Failed to close client socket\n";
    }


    rc = close(p.serverFd->getFd());
    if(rc != 0)
    {
        std::cerr << "Failed to close server socket\n";
    }
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

    TSLOG("addr %s port %s", addr.c_str(), port.c_str());

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


}