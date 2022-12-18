#include "util.hpp"

#include "stream.hpp"
#include "dgram.hpp"
#include "Logger.hpp"

#include <iostream>
#include <cstring>

#include <sys/socket.h>
#include <linux/net_tstamp.h>
#include <unistd.h>

namespace ts
{

void raiseError( const char* x)
{
    std::cerr << x << "\n last error = " << strerror(errno) << std::endl;
    exit(1);
}

SocketPair createSocketPair(Transport t)
{
    SocketPair p;
    if(t == Transport::TCP)
    {
        p = createTcpPair();
        std::cout << "TCP socket pair created\n";
    }
    else if(t == Transport::TCP_LOCAL)
    {
        p = createStreamLocalPair();
        std::cout << "TCP_LOCAL socket pair created\n";
    }
    else if(t == Transport::UDP)
    {
        p = createUdpPair();
        std::cout << "UDP socket pair created\n";
    }
    else if(t == Transport::UDP_LOCAL)
    {
        p = createDgramLocalPair();
        std::cout << "UDP_LOCAL socket pair created\n";
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
        raiseError("socket creation error");

    
    uint32_t val = 1;

    if(family == AF_INET)
    {
        val =  
            // SOF_TIMESTAMPING_TX_HARDWARE
              SOF_TIMESTAMPING_TX_SOFTWARE |

             SOF_TIMESTAMPING_RX_SOFTWARE |
            // | SOF_TIMESTAMPING_RX_HARDWARE |

            SOF_TIMESTAMPING_SOFTWARE 
            // | SOF_TIMESTAMPING_RAW_HARDWARE
            ;
    }
    
    int res = setsockopt(fd, SOL_SOCKET, (family == AF_INET) ? SO_TIMESTAMPING : SO_TIMESTAMPNS, &val, sizeof(val));

    if(res != 0)
    {
        raiseError("Failed to setsockopt");
    }

    int v = 1;
    res = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(v));
    if(res != 0)
    {
        raiseError("Failed to setsockopt");
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

}