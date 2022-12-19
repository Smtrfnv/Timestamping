#include "Endpoint.hpp"

#include "Sender.hpp"
#include "Receiver.hpp"
#include "Logger.hpp"

namespace ts
{

void Endpoint::start(const SocketPair& p, Mode mode, const Params& par)
{
    if(mode == Mode::TX)
    {
        TSLOG("Starting Endpoint in TX mode");

        auto l = [&](){

            pthread_setname_np(pthread_self(), "TX thread"); 

            Sender s(p.clientFd);
            Sender::Params params = {};
            params.receiveraddr = p.serveraddr;
            params.msToSleep = par.msToSleep;
            params.sendBufferSize = par.sendBufferSize;
            params.mode = Sender::Mode::LargePackets;
            s.start(params);
        };
        trd = std::thread(l);
    }
    else if(mode == Mode::RX)
    {
        TSLOG("Starting Endpoint in RX mode");

        auto l = [&](){

            pthread_setname_np(pthread_self(), "RX thread"); 

            Receiver r(p.serverFd);
            Receiver::Params params = {};
            params.bufferCapacity = 10000;
            r.start(params);
        };
        trd = std::thread(l);
    }
}


}
                                                                                  