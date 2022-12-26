#pragma once

#include "SocketWrapper.hpp"

#include <memory>
#include <variant>
#include <vector>

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
    void consumeTimer() const;
    void performSend(int iteration, const Params& p);

    
    std::shared_ptr<SocketWrapper> socket;
    std::vector<char> sendBuf;
    uint8_t* ctrlMsgBuf; //must be allocated aligned
    const size_t ctrlMsgBufSize;
    
    int epollFd;
    int timerFd;

};

}