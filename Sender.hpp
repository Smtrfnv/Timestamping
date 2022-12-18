#pragma once

#include "SocketWrapper.hpp"

#include <memory>
#include <variant>

#include <netinet/in.h>
#include <sys/un.h>


namespace ts
{


class Sender
{

public:

    enum class Mode
    {
        SmallPackets,
        LargePackets
    };

    struct Params
    {

        std::variant<sockaddr_in, sockaddr_un> receiveraddr;
        int msToSleep;
        Mode mode;
        int sendBufferSize;
    };

    Sender(const std::shared_ptr<SocketWrapper>& s);
    ~Sender();

    void start(const Params&);

private:

    void incrementalSend(const Params&);

    void createEpollAndTimer(const Params&);

    
    std::shared_ptr<SocketWrapper> socket;
    
    int epollFd;
    int timerFd;

};

}