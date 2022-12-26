#include <iostream>
#include <array>
#include <thread>
#include <chrono>
#include <cstring>
#include <unistd.h>
#include <array>

#include <signal.h>

#include "util.hpp"
#include "Logger.hpp"
#include "Endpoint.hpp"
#include "ConfigParser.hpp"

namespace ts
{

void signalHandler(int s)
{
    printf("Caught signal %d\n",s);
    exit(1); 
}

void registerSigintHandler()
{
    struct sigaction sigIntHandler = {};
    sigIntHandler.sa_handler = ts::signalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
}


struct Params
{
    bool enableLog = false;
};

Params parseOptions(int argc, char* argv[])
{
    Params p;
    int c;
    while ((c = getopt(argc, argv, "l")) != -1)
    {
        switch(c)
        {
            case('l'):
            {
                std::cout << "got option l\n";
                p.enableLog = true;
            } break;

            default:
            {
                std::cerr << "unsupported option " << c << std::endl;
            }
        }
    }

    return p;
}


} //namespace ts


int main(int argc, char* argv[])
{
    using namespace ts;

    Params param = parseOptions(argc, argv);

    if(param.enableLog) Logger::enable();
    else Logger::disable();

    // udpScenario();
    // tcpScenario();
    // udpLocalScenario();
    // tcpLocalScenario();

    ConfigParser parser;
    const auto descriptions = parser.parseConfigFile("../config.json");

    std::vector<std::shared_ptr<EndpointNew>> endpoints;
    for(const auto& d: descriptions)
    {
        TSLOG("%s", toString(d).c_str());        
        std::shared_ptr<EndpointNew> ep = std::make_shared<EndpointNew>(d);
        endpoints.emplace_back(std::move(ep));
    }

    for(auto& e: endpoints)
    {
        e->start();
    }

    for(auto& e: endpoints)
    {
        e->waitReadtyToOperate();
    }

    for(auto& e: endpoints)
    {
        e->startTask();
    }

    for(auto& e: endpoints)
    {
        e->wait();
    }







    TSLOG("DONE");
}