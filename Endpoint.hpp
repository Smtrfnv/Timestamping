#pragma once

#include "SocketPair.hpp"

#include <thread>

namespace ts
{

class Endpoint
{

public:
    
    enum class Mode
    {
        TX,
        RX
    };

    void start(const SocketPair& p, Mode mode);

    void wait() { trd.join(); }

private:

    std::thread trd;

};


}