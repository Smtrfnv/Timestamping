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

    struct Params
    {
        std::variant<sockaddr_in, sockaddr_un> receiveraddr;
        int msToSleep;
    };

    Sender(const std::shared_ptr<SocketWrapper>& s);

    void start(const Params&);

private:

    void incrementalSend(const Params&) const;

    std::shared_ptr<SocketWrapper> socket;

};

}