#pragma once

#include "Transport.hpp"
#include "EndpointTask.hpp"

#include <string>

namespace ts
{

struct EndpointDescription
{
    std::string name;
    Transport transport;
    std::string selfAddr;
    std::string peerAddr; // valid only for connection protocols

    Task task;
};

}
