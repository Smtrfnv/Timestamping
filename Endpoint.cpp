#include "Endpoint.hpp"

#include "Sender.hpp"
#include "Receiver.hpp"
#include "Logger.hpp"

namespace ts
{

void Endpoint::start(const SocketPair& p, Mode mode)
{
    if(mode == Mode::TX)
    {
        TSLOG("Starting Endpoint in TX mode");

        auto l = [&](){
            Sender s(p.clientFd);
            Sender::Params params = {};
            params.receiveraddr = p.serveraddr;
            params.msToSleep = 2000;
            s.start(params);
        };
        trd = std::thread(l);
    }
    else if(mode == Mode::RX)
    {
        TSLOG("Starting Endpoint in RX mode");

        auto l = [&](){
            Receiver r(p.serverFd);
            Receiver::Params params = {};
            params.bufferCapacity = 1024;
            r.start(params);
        };
        trd = std::thread(l);
    }
}


}
                                                                                  