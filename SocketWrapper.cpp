#include "SocketWrapper.hpp"

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
    if(fd == 0)
        return;
    
    if(t == Transport::TCP)
    {
        if(shutdown(fd, SHUT_RDWR) != 0)
        {
            std::cerr << "Failed to shutdown " << name << "socket\n";
        }
        std::cout << "Shutdown " << name << " socket" << std::endl;

        char buf[10];
        int n = 0;
        do
        {
            n = read(fd, buf, 10);
        } while(n != 0 && n != -1);
    }

    if(close(fd) != 0)
    {
        std::cerr << "Failed to close " << name << "socket\n";
    }
    std::cout << "Closed  " << name << " socket" << std::endl;
}

}