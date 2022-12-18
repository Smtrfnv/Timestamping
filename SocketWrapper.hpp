#pragma once

#include "Transport.hpp"

namespace ts
{

class SocketWrapper
{
public:

    // SocketWrapper() = default;
    // SocketWrapper(int family, int type, int protocol);
    SocketWrapper(const char* _name, int _fd, Transport _t);
    ~SocketWrapper();

    int getFd() const { return fd; }
    Transport getTransport() const { return t; }
    // int getFamily() const { return family; }
    // int getType() const { return type; }
    // int getProtocol() const {return protocol; }

private:

    SocketWrapper(const SocketWrapper&) = delete;
    SocketWrapper(SocketWrapper&&) = delete;
    SocketWrapper& operator=(const SocketWrapper&) = delete;
    SocketWrapper& operator=(SocketWrapper&&) = delete;

    // int family;
    // int type;
    // int protocol;

    const char* name;
    Transport t;
    int fd;

};

}