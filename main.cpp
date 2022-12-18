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

Transport getTransportType(const char * str)
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

} //namespace ts


int main(int argc, char* argv[])
{
    using namespace ts;

    Logger::enable();
    TSLOG("Hello from logger");

    if(argc != 2)
    {
        raiseError("Incorrect number of arguments, should be 1");
    }

    const Transport transport = getTransportType(argv[1]);

    registerSigintHandler();

    const SocketPair p = createSocketPair(transport);

    Endpoint tx;
    Endpoint rx;
    
    rx.start(p, Endpoint::Mode::RX);
    tx.start(p, Endpoint::Mode::TX);

    rx.wait();
    tx.wait();
}