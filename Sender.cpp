#include "Sender.hpp"

#include "Logger.hpp"
#include "ctrlMsgUtils.hpp"

#include <stdio.h>
#include <cstring>
#include <chrono>
#include <thread>

#include <sys/socket.h>

namespace ts
{

Sender::Sender(const std::shared_ptr<SocketWrapper>& s) : socket(s)
{}

void Sender::start(const Params& p)
{
    incrementalSend(p);    
}

void Sender::incrementalSend(const Params& p) const
{
    const size_t BUFSIZE = 100;
    char buf[BUFSIZE];

    for(int i = 0;;++i)
    {
        snprintf(buf, BUFSIZE, "%d", i);

        TSLOG("sending [%s]", buf);

        const size_t sendLen = strlen(buf);
        
        int n = 0;
        Transport t = socket->getTransport();
        if(t == Transport::TCP || t == Transport::TCP_LOCAL)
        {
            n = send(socket->getFd(), buf, sendLen, 0);
        }
        else if(t == Transport::UDP)
        {
            n = sendto(socket->getFd(), buf, sendLen, 0, (sockaddr*) &std::get<sockaddr_in>(p.receiveraddr), sizeof(sockaddr_in));
        }
        else if(t == Transport::UDP_LOCAL)
        {
            n = sendto(socket->getFd(), buf, sendLen, 0, (sockaddr*)  &std::get<sockaddr_un>(p.receiveraddr), sizeof(sockaddr_un));
        }

        if(n == -1)
        {
            TSLOG("Failed to send");
            continue;
        }

        TSLOG("sent %d bytes", n);

        

        // if(ctr % 2)
        // {
        //     std::array<uint8_t, MAXLEN * 2> packetBuf = {};

        //     recvCtrlMessageTx(p, packetBuf.begin(), packetBuf.size());
        //     recvCtrlMessageTx(p, packetBuf.begin(), packetBuf.size());
        // }
        // ctr++;

        recvCtrlMessageTx(socket->getFd());
    
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(std::chrono::milliseconds(p.msToSleep));
        }
    }
}

}