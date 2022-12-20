#pragma once

#include "Endpoint.hpp"

#include "EndpointDescription.hpp"

#include <memory>

namespace ts
{

class EndpointFactory
{

public:

    std::shared_ptr<EndpointNew> createEndpoint(const EndpointDescription& ed) const;

private:

};

}
