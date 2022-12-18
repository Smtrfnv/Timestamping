#include "Sender.hpp"

#include "Logger.hpp"
#include "ctrlMsgUtils.hpp"
#include "util.hpp"

#include <stdio.h>
#include <cstring>
#include <chrono>
#include <thread>
#include <vector>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <unistd.h>

namespace ts
{

Sender::Sender(const std::shared_ptr<SocketWrapper>& s) : socket(s), epollFd(0), timerFd(0)
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
    std::vector<char> buf(p.sendBufferSize, 0);

    createEpollAndTimer(p);

    const size_t MAX_EVENTS = 10;
    epoll_event epevents[MAX_EVENTS] = {};

    for(int i = 0;;++i)
    {
        snprintf(buf.data(), buf.size(), "%d", i);

        TSLOG("sending [%s]", buf.data());

        const size_t sendLen = p.mode == Mode::SmallPackets ? strlen(buf.data()) : buf.size();
        
        int n = 0;
        Transport t = socket->getTransport();
        if(t == Transport::TCP || t == Transport::TCP_LOCAL)
        {
            n = send(socket->getFd(), buf.data(), sendLen, 0);
        }
        else if(t == Transport::UDP)
        {
            n = sendto(socket->getFd(), buf.data(), sendLen, 0, (sockaddr*) &std::get<sockaddr_in>(p.receiveraddr), sizeof(sockaddr_in));
        }
        else if(t == Transport::UDP_LOCAL)
        {
            n = sendto(socket->getFd(), buf.data(), sendLen, 0, (sockaddr*)  &std::get<sockaddr_un>(p.receiveraddr), sizeof(sockaddr_un));
        }

        if(n == -1)
        {
            TSLOG("Failed to send");
            continue;
        }

        TSLOG("sent %d bytes out of %d", n, sendLen);

        recvCtrlMessageTx(socket->getFd());
    
        // {
        //     using namespace std::chrono_literals;
        //     std::this_thread::sleep_for(std::chrono::milliseconds(p.msToSleep));
        // }

        const int res = epoll_wait(epollFd, epevents, MAX_EVENTS, -1);
        if(res == -1)
        {
            TSLOG("epoll_wait error %d : %s", res, strerror(errno));
        }
        else
        {
            TSLOG("epoll_wait result: %d", res);
            //right now it is possible only to read a timer

            ssize_t expired = 0;
            int res = read(timerFd, &expired, sizeof(expired));
            if(res == -1)
            {
                raiseError("Failed to read from timer");
            }
            TSLOG("Timer expired %d times", expired);
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

    epoll_event epevent = {};
    epevent.events = EPOLLIN;


    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, timerFd, &epevent) != 0)
    {
        raiseError("Failed to add timerFd to epoll interest list");
    }
}

}