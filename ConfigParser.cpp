#include "ConfigParser.hpp"

#include "Logger.hpp"
#include "Transport.hpp"

#include <fstream>
#include <string>

namespace ts
{


std::vector<EndpointDescription> ConfigParser::parseConfigFile(const char* fname)
{
    using namespace rapidjson;

    std::vector<EndpointDescription> epDescriptions;

    TSLOG("parsing config file: %s", fname);

    std::ifstream file;
    file.open(fname);
    
    if(!file.is_open())
    {
        TSLOG("Failed to open config file");
        exit(1);
    }

    std::string fileContent((std::istreambuf_iterator<char>(file)),
                 std::istreambuf_iterator<char>());

    TSLOG("config internals:\n%s", fileContent.c_str());
 
    Document d;
    d.Parse(fileContent.c_str());

    const char* ENDPOINTS = "Endpoints";

    if(!d.HasMember(ENDPOINTS))
    {
        TSLOG("No %s object", ENDPOINTS);
        exit(1);
    }

    const Value& endpoints = d[ENDPOINTS];

    if(!endpoints.IsArray())
    {
        TSLOG("%s is not array", ENDPOINTS);
        exit(1);
    }

    if(endpoints.Size() == 0)
    {
        TSLOG("no %s configured", ENDPOINTS);
        exit(1);
    }

    for (SizeType i = 0; i < endpoints.Size(); i++)
    {
        const Value& v = endpoints[i];

        EndpointDescription epD = {};

        if(!parseEndpoint(epD, v))
        {
            TSLOG("failed to parse endpoint");
            exit(1);
        }

        epDescriptions.push_back(epD);
    }

    return epDescriptions;
}

bool ConfigParser::parseEndpoint(EndpointDescription& endpoint, const rapidjson::Value& v)
{
    const char* NAME = "name";
    const char* TRANSPORT = "transport";
    const char* SELFADDR = "selfAddr";
    const char* PEERADDR = "peerAddr";

    endpoint = {};

    if(!v.HasMember(NAME))
    {
        TSLOG("No %s member", NAME);
        return false;
    }
    endpoint.name = v[NAME].GetString();

    if(!v.HasMember(TRANSPORT))
    {
        TSLOG("No %s member", TRANSPORT);
        return false;
    }
    endpoint.transport = stringToTransport(v[TRANSPORT].GetString());

    if(v.HasMember(SELFADDR))
    {
        endpoint.selfAddr = v[SELFADDR].GetString();
    }
    if(v.HasMember(PEERADDR))
    {
        endpoint.peerAddr = v[PEERADDR].GetString();
    }
    return true;
}

}