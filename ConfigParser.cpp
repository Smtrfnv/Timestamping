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
    using namespace rapidjson;

    const char* NAME = "name";
    const char* TRANSPORT = "transport";
    const char* SELFADDR = "selfAddr";
    const char* PEERADDR = "peerAddr";
    const char* TASK = "task";
    const char* SOCKOPTS = "sockopts";

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

    if(!v.HasMember(TASK))
    {
        TSLOG("No %s member", TASK);
        return false;        
    }

    if(v.HasMember(SOCKOPTS))
    {
        const Value& sockopts = v[SOCKOPTS];
        if(!sockopts.IsArray())
        {
            TSLOG("%s must be an array", SOCKOPTS);
            return false;
        }
        if(sockopts.Size() == 0)
        {
            TSLOG("%s must contain at least one option (if present)", SOCKOPTS);
            return false;
        }
        bool res = parseSockopts(endpoint.sockOpts, sockopts);
        if(!res)
        {
            TSLOG("Failed to parse %s", SOCKOPTS);
            return false;
        }
    }

    const Value& tasks = v[TASK];
    if(!tasks.IsArray())
    {
        TSLOG("%s is not array", TASK);
        return false; 
    }
    if(tasks.Size() != 1)
    {
        TSLOG("Wrong number of configured %s", TASK);
        return false;
    }

    if(!parseTask(endpoint.task, tasks[0]))
    {
        return false;
    }

    return true;
}

bool ConfigParser::parseTask(Task& task, const rapidjson::Value& v)
{
    task = {};

    const char* MODE = "mode";
    const char* MSTOSLEEP = "msToSleep";
    const char* PACKETSIZE = "packetSize";
    const char* TARGETADDR = "targetAddr";
    const char* RECVBUFSIZE = "recvBufSize";
    
    if(!v.HasMember(MODE))
    {
        TSLOG("No %s member", MODE);
        return false;
    }

    const std::string mode = v[MODE].GetString();
    task.mode = stringToMode(mode); //todo: catch exception

    if(v.HasMember(MSTOSLEEP))
    {
        task.msToSleep = v[MSTOSLEEP].GetInt();
    }

    if(v.HasMember(PACKETSIZE))
    {
        task.packetSize = v[PACKETSIZE].GetInt();
    }

    if(v.HasMember(TARGETADDR))
    {
        task.targetaddr = v[TARGETADDR].GetString();
    }

    if(v.HasMember(RECVBUFSIZE))
    {
        task.recvBufSize = v[RECVBUFSIZE].GetInt();
    }

    return true;
}

bool ConfigParser::parseSockopts(SocketOptions& opts, const rapidjson::Value& v)
{
    opts = {};
    for(size_t i = 0; i < v.Size(); ++i)
    {
        if(!opts.set(stringToSockOption(v[i].GetString())))
        {
            TSLOG("Failed to collect option %s", v[i].GetString());
            return false;
        }
    }
    return true;
}

}