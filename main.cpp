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

Transport getTransportType(char * str)
{
    if(strcmp("stream", str) == 0)
    {
        return Transport::STREAM;
    }
    if(strcmp("dgram", str) == 0)
    {
        return Transport::DGRAM;
    }
    if(strcmp("stream_local", str) == 0)
    {
        return Transport::STREAM_LOCAL;
    }
    if(strcmp("dgram_local", str) == 0)
    {
        return Transport::DGRAM_LOCAL;
    }
    raiseError("Unsupported transport type");
    return Transport::DGRAM;
}

struct Params
{
    Transport transport = Transport::DGRAM;
    bool enableLog = false;
    int timeToSleepMs = 2000;
};

Params parseOptions(int argc, char* argv[])
{
    Params p;
    int c;
    while ((c = getopt(argc, argv, "t:ls:")) != -1)
    {
        switch(c)
        {
            case('t'):
            {
                std::cout << "got option t\n";
                Transport t = getTransportType(optarg);
                std::cout << toString(t) << std::endl;

                p.transport = t;

            } break;

            case('l'):
            {
                std::cout << "got option l\n";
                p.enableLog = true;
            } break;

            case('s'):
            {
                std::cout << "got option s\n";
                int time = atoi(optarg);
                p.timeToSleepMs = time;
                std::cout << "tts " << p.timeToSleepMs << std::endl;
            }

            default:
            {
                std::cerr << "unsupported option " << c << std::endl;
            }
        }
    }

    return p;
}


void udpScenario()
{
    EndpointDescription recvD;
    recvD.name = "endpoint udp rx";
    recvD.selfAddr = "127.0.0.1:2552";
    recvD.transport = Transport::DGRAM;

    EndpointNew rxEndp(recvD);


    
    EndpointDescription sendD;
    sendD.name = "endpoint udp tx";
    sendD.transport = Transport::DGRAM;

    EndpointNew txEndp(sendD);


    rxEndp.start();
    txEndp.start();
    rxEndp.waitReadtyToOperate();
    txEndp.waitReadtyToOperate();


    {
        EndpointNew::Task rxTask = {};
        rxTask.mode = EndpointNew::Mode::RX;
        rxEndp.startTask(rxTask);
    }

    {
        EndpointNew::Task txTask = {};
        txTask.mode = EndpointNew::Mode::TX;
        txTask.msToSleep = 2000;
        txTask.sendBufferSize = 1024;
        txTask.targetaddr = rxEndp.getAddr();

        txEndp.startTask(txTask);
    }

    txEndp.wait();
    rxEndp.wait();
}

void tcpScenario()
{
    std::string servAddr = "127.0.0.1:2553";
    EndpointDescription recvD;
    recvD.name = "endpoint tcp rx";
    recvD.selfAddr = servAddr;
    recvD.transport = Transport::STREAM;

    EndpointNew rxEndp(recvD);

    EndpointDescription sendD;
    sendD.name = "endpoint tcp tx";
    sendD.peerAddr = servAddr;
    sendD.transport = Transport::STREAM;

    EndpointNew txEndp(sendD);

    rxEndp.start();

    txEndp.start();
    rxEndp.waitReadtyToOperate();
    txEndp.waitReadtyToOperate();



    {
        EndpointNew::Task rxTask = {};
        rxTask.mode = EndpointNew::Mode::RX;
        rxEndp.startTask(rxTask);
    }

    {
        EndpointNew::Task txTask = {};
        txTask.mode = EndpointNew::Mode::TX;
        txTask.msToSleep = 2000;
        txTask.sendBufferSize = 1024;
        txTask.targetaddr = rxEndp.getAddr();

        txEndp.startTask(txTask);
    }

    txEndp.wait();
    rxEndp.wait();
}

void tcpLocalScenario()
{
    std::string servAddr = "tcpLocal";
    EndpointDescription recvD;
    recvD.name = "endpoint tcp local rx";
    recvD.selfAddr = servAddr;
    recvD.transport = Transport::STREAM_LOCAL;

    EndpointNew rxEndp(recvD);

    EndpointDescription sendD;
    sendD.name = "endpoint tcp local tx";
    sendD.peerAddr = servAddr;
    sendD.transport = Transport::STREAM_LOCAL;

    EndpointNew txEndp(sendD);

    rxEndp.start();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    txEndp.start();
    rxEndp.waitReadtyToOperate();
    txEndp.waitReadtyToOperate();



    {
        EndpointNew::Task rxTask = {};
        rxTask.mode = EndpointNew::Mode::RX;
        rxEndp.startTask(rxTask);
    }

    {
        EndpointNew::Task txTask = {};
        txTask.mode = EndpointNew::Mode::TX;
        txTask.msToSleep = 2000;
        txTask.sendBufferSize = 1024;
        txTask.targetaddr = rxEndp.getAddr();

        txEndp.startTask(txTask);
    }

    txEndp.wait();
    rxEndp.wait();
}

void udpLocalScenario()
{
    EndpointDescription recvD;
    recvD.name = "endpoint udp local rx";
    recvD.selfAddr = "udpLocal";
    recvD.transport = Transport::DGRAM_LOCAL;

    EndpointNew rxEndp(recvD);


    
    EndpointDescription sendD;
    sendD.name = "endpoint udp local tx";
    sendD.transport = Transport::DGRAM_LOCAL;

    EndpointNew txEndp(sendD);


    rxEndp.start();
    txEndp.start();
    rxEndp.waitReadtyToOperate();
    txEndp.waitReadtyToOperate();


    {
        EndpointNew::Task rxTask = {};
        rxTask.mode = EndpointNew::Mode::RX;
        rxEndp.startTask(rxTask);
    }

    {
        EndpointNew::Task txTask = {};
        txTask.mode = EndpointNew::Mode::TX;
        txTask.msToSleep = 2000;
        txTask.sendBufferSize = 1024;
        txTask.targetaddr = rxEndp.getAddr();

        txEndp.startTask(txTask);
    }

    txEndp.wait();
    rxEndp.wait();    
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





    TSLOG("DONE");
}