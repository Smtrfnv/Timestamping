#include "util.hpp"

#include "stream.hpp"
#include "dgram.hpp"
#include "Logger.hpp"

#include <vector>

#include <sys/socket.h>
#include <linux/net_tstamp.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/eventpoll.h>

namespace ts
{

int createSocket(int family, int type, int protocol)
{
    TSLOG("creating socket");

    int fd = socket(family, type, protocol);
    if(fd < 0)
    {
        TSLOG("socket creation error");
        return -1;
    }

    int bufSize = 0;
    socklen_t len = sizeof(bufSize);

    if(getsockopt(fd,SOL_SOCKET, SO_SNDBUF, &bufSize, &len) == 0)
    {
        TSLOG("send buffer size is %d", bufSize);
    }
    
    bufSize = 0;
    len = sizeof(bufSize);

    if(getsockopt(fd,SOL_SOCKET, SO_RCVBUF, &bufSize, &len) == 0)
    {
        TSLOG("recv buffer size is %d", bufSize);
    }



    return fd;
}

bool setOptions(int fd, int family, const SocketOptions& opts)
{
    const auto name = opts.getName();
    const auto val = opts.get();
    
    int res = setsockopt(fd, SOL_SOCKET, name, &val, sizeof(val));

    if(res != 0)
    {
        TSLOG("Failed to setsockopt");
        return false;
    }
    return true;
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


const std::vector<std::pair<uint64_t, std::string>> epollEventsToStringTable = {
        {EPOLLIN, "EPOLLIN"},
        {EPOLLPRI, "EPOLLPRI"},
        {EPOLLOUT, "EPOLLOUT"},
        {EPOLLRDNORM, "EPOLLRDNORM"},
        {EPOLLRDBAND, "EPOLLRDBAND"},
        {EPOLLWRNORM, "EPOLLWRNORM"},
        {EPOLLWRBAND, "EPOLLWRBAND"},
        {EPOLLMSG, "EPOLLMSG"},
        {EPOLLERR, "EPOLLERR"},
        {EPOLLHUP, "EPOLLHUP"},
        {EPOLLRDHUP, "EPOLLRDHUP"},
        {EPOLLEXCLUSIVE, "EPOLLEXCLUSIVE"},
        {EPOLLWAKEUP, "EPOLLWAKEUP"},
        {EPOLLONESHOT, "EPOLLONESHOT"},
        {EPOLLET, "EPOLLET"}
};

std::string epollEventsToString(uint64_t epollEvents)
{
    std::stringstream ss;
    for(size_t i = 0; i < epollEventsToStringTable.size(); ++i)
    {
        if(epollEvents & epollEventsToStringTable[i].first)
            ss << epollEventsToStringTable[i].second << " ";
        
    }

    return ss.str();
}


}