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

    struct Params //common structure to store parameter for both rx & tx
    {
        // TX
        int msToSleep = 2000;
        int sendBufferSize = 1024;

        // RX

    };

    void start(const SocketPair& p, Mode mode, const Params&);

    void wait() { trd.join(); }

private:

    std::thread trd;

};


}