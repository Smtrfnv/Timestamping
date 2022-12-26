#include "util.hpp"

#include "stream.hpp"
#include "dgram.hpp"
#include "Logger.hpp"

#include <vector>

#include <sys/socket.h>
#include <linux/net_tstamp.h>
#include <unistd.h>
#include <arpa/inet.h>

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
    //TODO: update the whole logic so that timestamping and timestampns are possible to set
    uint32_t val = 1;

    if(family == AF_INET)
    {
        val = opts.get();
    }
    
    int res = setsockopt(fd, SOL_SOCKET, (family == AF_INET) ? SO_TIMESTAMPING : SO_TIMESTAMPNS, &val, sizeof(val));

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


}