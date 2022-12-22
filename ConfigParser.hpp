#pragma once

#include "EndpointDescription.hpp"
#include <vector>

#include <rapidjson/document.h>

namespace ts
{

class ConfigParser
{

public:
    ConfigParser() = default;
    std::vector<EndpointDescription> parseConfigFile(const char* fname);

private:

    bool parseEndpoint(EndpointDescription& endpoint, const rapidjson::Value& v);

};

}