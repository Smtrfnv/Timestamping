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
    if(strcmp("tcp", str) == 0)
    {
        return Transport::TCP;
    }
    if(strcmp("udp", str) == 0)
    {
        return Transport::UDP;
    }
    if(strcmp("tcp_local", str) == 0)
    {
        return Transport::TCP_LOCAL;
    }
    if(strcmp("udp_local", str) == 0)
    {
        return Transport::UDP_LOCAL;
    }
    raiseError("Unsupported transport type");
    return Transport::TCP;
}

struct Params
{
    Transport transport = Transport::UDP;
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
    recvD.transport = Transport::UDP;

    EndpointNew rxEndp(recvD);


    
    EndpointDescription sendD;
    sendD.name = "endpoint udp tx";
    sendD.transport = Transport::UDP;

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
    recvD.transport = Transport::TCP;

    EndpointNew rxEndp(recvD);

    EndpointDescription sendD;
    sendD.name = "endpoint tcp tx";
    sendD.peerAddr = servAddr;
    sendD.transport = Transport::TCP;

    EndpointNew txEndp(sendD);

    rxEndp.start();
    txEndp.start();
    rxEndp.waitReadtyToOperate();
    txEndp.waitReadtyToOperate();
}

} //namespace ts


int main(int argc, char* argv[])
{
    using namespace ts;

    Params param = parseOptions(argc, argv);


    if(param.enableLog) Logger::enable();
    else Logger::disable();

    TSLOG("Hello from logger");
#if 0
    bool udpSpamMode = false;

    registerSigintHandler();

    const SocketPair p = createSocketPair(param.transport);

    Endpoint::Params epPar;
    epPar.msToSleep = param.timeToSleepMs;

    if(udpSpamMode == false)
    {
        Endpoint tx;
        Endpoint rx;

    
        rx.start(p, Endpoint::Mode::RX, epPar);
        tx.start(p, Endpoint::Mode::TX, epPar);

        rx.wait();
        tx.wait();
    }
    else
    {
        Endpoint tx;
        tx.start(p, Endpoint::Mode::TX, epPar);
        tx.wait();
    }
#endif

    // createSocketPair(Transport::UDP);

    // udpScenario();
    tcpScenario();



    TSLOG("DONE");
}