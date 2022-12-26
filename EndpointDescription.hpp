#pragma once

#include "Transport.hpp"
#include "EndpointTask.hpp"
#include "SocketOptions.hpp"

#include <iostream>
#include <string>
#include <sstream>

namespace ts
{

struct EndpointDescription
{
    std::string name;
    Transport transport;
    std::string selfAddr;
    std::string peerAddr; // valid only for connection protocols

    SocketOptions sockOpts;

    Task task;

    friend std::ostream& operator<<(std::ostream& os, const EndpointDescription& d);
};

inline
std::ostream& operator<<(std::ostream& os, const EndpointDescription& d)
{
    std::stringstream ss;

    ss << "Name: " << d.name << "; transport: " << toString(d.transport) << "; selfAddr: " << d.selfAddr << "; peerAddr: " << d.peerAddr <<
        "; sockopts: " << d.sockOpts << "; task: " << d.task;

    os << ss.str();

    return os;
}

inline
std::string toString(const EndpointDescription& d)
{
    std::stringstream ss;
    ss << d;
    return ss.str();
}

}
