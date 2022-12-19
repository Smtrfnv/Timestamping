#include "Sender.hpp"

#include "Logger.hpp"
#include "ctrlMsgUtils.hpp"
#include "util.hpp"

#include <stdio.h>
#include <cstring>
#include <chrono>
#include <thread>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <unistd.h>

namespace ts
{

Sender::Sender(const std::shared_ptr<SocketWrapper>& s) : socket(s), sendBuf(), epollFd(0), timerFd(0)
{}

Sender::~Sender()
{
    if(timerFd)
    {
        close(timerFd);
    }
    if(epollFd)
    {
        close(epollFd);
    }
}

void Sender::start(const Params& p)
{
    incrementalSend(p);
}

void Sender::incrementalSend(const Params& p)
{
    sendBuf.resize(p.sendBufferSize);

    createEpollAndTimer(p);

    const size_t MAX_EVENTS = 10;
    epoll_event epevents[MAX_EVENTS] = {};

    int pktNum = 0;

    while(true)
    {
        if(p.msToSleep != 0)
        {
            const int numEvents = epoll_wait(epollFd, epevents, MAX_EVENTS, -1);
            if(numEvents == -1)
            {
                raiseError("epoll_wait error %d : %s", numEvents, strerror(errno));
            }

            TSLOG("epoll_wait result: %d", numEvents);

            for(int i = 0; i < numEvents; ++i)
            {
                if(epevents[i].data.fd == timerFd)
                {
                    // consume timer and do send
                    consumeTimer();
                    performSend(pktNum, p);
                    recvCtrlMessageTx(socket->getFd());
                    ++pktNum;
                }
                else if(epevents[i].data.fd == socket->getFd())
                {
                    //read msgerror queue
                    recvCtrlMessageTx(socket->getFd());
                }
                else
                {
                    raiseError("Incorrect epoll event received");
                }
            }
        }
        else
        {
            //special mode - infinite loop without polling
            performSend(pktNum, p);
            recvCtrlMessageTx(socket->getFd());
            ++pktNum;
        }
    }
}

void Sender::createEpollAndTimer(const Params& p)
{
    epollFd = epoll_create1(0);
    if(epollFd == -1)
    {
        raiseError("Failed to create epoll instance");
    }
    
    timerFd = timerfd_create(CLOCK_MONOTONIC, 0);
    if(timerFd == -1)
    {
        raiseError("Failed to create timerFd instance");
    }
    itimerspec timerspec = {};
    timerspec.it_interval.tv_sec = p.msToSleep / 1000;
    timerspec.it_interval.tv_nsec = (p.msToSleep % 1000) * 1000000;
    timerspec.it_value.tv_sec = 1;
    timerspec.it_value.tv_nsec = 0;    

    if(timerfd_settime(timerFd, 0, &timerspec, 0) != 0)
    {
        raiseError("Failed to start timer");
    }

    {
        epoll_event epevent = {};
        epevent.events = EPOLLIN;
        epevent.data.fd = timerFd;
        if(epoll_ctl(epollFd, EPOLL_CTL_ADD, timerFd, &epevent) != 0)
        {
            raiseError("Failed to add timerFd to epoll interest list");
        }
    }

    {
        epoll_event epevent = {};
        epevent.events = EPOLLERR;
        epevent.data.fd = socket->getFd();
        if(epoll_ctl(epollFd, EPOLL_CTL_ADD, socket->getFd(), &epevent) != 0)
        {
            raiseError("Failed to add sockFd to epoll interest list");
        }
    }
}

void Sender::consumeTimer() const
{
    ssize_t expired = 0;
    int res = read(timerFd, &expired, sizeof(expired));
    if(res == -1)
    {
        raiseError("Failed to read from timer");
    }
    TSLOG("Timer expired %d times", expired);
}

void Sender::performSend(int packetNum, const Params& p)
{
    snprintf(sendBuf.data(), sendBuf.size(), "%d", packetNum);

    TSLOG("sending [%s]", sendBuf.data());

    const size_t sendLen = p.mode == Mode::SmallPackets ? strlen(sendBuf.data()) : sendBuf.size();
    
    int n = 0;
    Transport t = socket->getTransport();
    if(t == Transport::TCP || t == Transport::TCP_LOCAL)
    {
        n = send(socket->getFd(), sendBuf.data(), sendLen, 0);
    }
    else if(t == Transport::UDP)
    {
        n = sendto(socket->getFd(), sendBuf.data(), sendLen, 0, (sockaddr*) &std::get<sockaddr_in>(p.receiveraddr), sizeof(sockaddr_in));
    }
    else if(t == Transport::UDP_LOCAL)
    {
        n = sendto(socket->getFd(), sendBuf.data(), sendLen, 0, (sockaddr*)  &std::get<sockaddr_un>(p.receiveraddr), sizeof(sockaddr_un));
    }

    if(n == -1)
    {
        TSLOG("Failed to send");
    }

    TSLOG("sent %d bytes out of %d", n, sendLen);
}

}