#include "SocketWrapper.hpp"

#include "Logger.hpp"

#include <sys/socket.h>
#include <iostream>
#include <unistd.h>

namespace ts
{

// SocketWrapper::SocketWrapper(int _family, int _type, int _protocol)
// {
    
// }

SocketWrapper::SocketWrapper(const char* _name, int _fd, Transport _t): name(_name)
{
    t = _t;
    fd = _fd;
}

SocketWrapper::~SocketWrapper()
{
    TSLOG("socket %s d-tor, fd = %d", name, fd);

    if(fd == 0)
    {
        return;
    }
    
    if(t == Transport::STREAM)
    {
        if(shutdown(fd, SHUT_RDWR) != 0)
        {
            TSLOG("socket %s, failed to shutdown", name);
        }
        TSLOG("socket %s, done shutdown", name);

        char buf[10];
        int n = 0;
        do
        {
            n = read(fd, buf, 10);
        } while(n != 0 && n != -1);
    }

    if(close(fd) != 0)
    {
        TSLOG("socket %s, failed to close", name);
    }
    TSLOG("socket %s, closed", name);
}

}