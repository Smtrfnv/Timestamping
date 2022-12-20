#pragma once

#include "Transport.hpp"

#include <string>

namespace ts
{

struct EndpointDescription
{
    std::string name;
    Transport transport;
    std::string selfAddr;
};

}
