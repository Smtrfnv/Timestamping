#include "util.hpp"

#include <iostream>
#include <cstring>

#include <sys/socket.h>
#include <linux/net_tstamp.h>
#include <linux/errqueue.h>

namespace ts
{

void raiseError( const char* x)
{
    std::cerr << x << std::endl;
    exit(1);
}

int createSocket(int family, int type, int protocol)
{
    std::cout << "creating socket\n";
    int fd = socket(family, type, protocol);
    if(fd < 0)
        raiseError("socket creation error");

    
    uint32_t val = 1;

    if(family == AF_INET)
    {
        val =  
            SOF_TIMESTAMPING_TX_HARDWARE
            | SOF_TIMESTAMPING_TX_SOFTWARE

            | SOF_TIMESTAMPING_RX_SOFTWARE
            | SOF_TIMESTAMPING_RX_HARDWARE

            | SOF_TIMESTAMPING_SOFTWARE
            | SOF_TIMESTAMPING_RAW_HARDWARE
            ;
    }
    
    int res = setsockopt(fd, SOL_SOCKET, (family == AF_INET) ? SO_TIMESTAMPING : SO_TIMESTAMPNS, &val, sizeof(val));

    if(res != 0)
    {
        raiseError("Failed to setsockopt");
    }

    return fd;
}

void processCtrlMessage(cmsghdr &chdr)
{
    std::cout << "cmsg_level = " << chdr.cmsg_level << " cmsg_type = " << chdr.cmsg_type << std::endl;
    
    switch(chdr.cmsg_level)
    {
        case(SOL_SOCKET):
        {
            switch(chdr.cmsg_type)
            {
                case(SO_TIMESTAMPING):
                {
                    std::cout << "Got SO_TIMESTAMPING\n";

                    scm_timestamping ts = {};

                    std::memcpy(&ts, CMSG_DATA(&chdr), sizeof(ts));

                    std::cout << "ts[0]" << ts.ts[0].tv_sec << " " << ts.ts[0].tv_nsec
                              << "\nts[1]" << ts.ts[1].tv_sec << " " << ts.ts[1].tv_nsec
                              << "\nts[2]" << ts.ts[2].tv_sec << " " << ts.ts[2].tv_nsec << std::endl;
                    break;
                }
                case(SO_TIMESTAMPNS):
                {
                    std::cout << "Got SO_TIMESTAMPNS\n";

                    timespec ts = {};
                    std::memcpy(&ts, CMSG_DATA(&chdr), sizeof(ts));
                    std::cout << "ts: " << ts.tv_sec << " " << ts.tv_nsec << std::endl;

                    break;
                }
                default:
                {
                    std::cout << "unsupported cmsg_type = " << chdr.cmsg_type << std::endl;
                }
            }
            break;
        }
        default:
        {
            std::cout << "Unknown cmsg_level\n";
        }
    }
}


}