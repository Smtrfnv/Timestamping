#pragma once

#include "Exception.hpp"

#include <iostream>
#include <string>
#include <sstream>

namespace ts
{

enum class Mode
{
    TX,
    RX
};

struct Task //common structure to store parameter for both rx & tx
{
    Mode mode;

    // TX
    int msToSleep = 2000;
    int packetSize = 1024;
    std::string targetaddr; //only for connectionless protocols

    // RX
    int recvBufSize = 1024;
};


inline
Mode stringToMode(const std::string& str)
{
    auto eq = [](auto a, auto b) { return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                                                        [](char a, char b) { return tolower(a) == tolower(b); }
                                                    );
                                 };

    const std::string tx = "tx";
    const std::string rx = "rx";

    if(eq(tx, str))
        return Mode::TX;
    
    if(eq(rx, str))
        return Mode::RX;

    throw Exception("Failed to covert string to mode");
}

inline
std::ostream& operator<<(std::ostream& os, const Task& t)
{
    std::stringstream ss;
    if(t.mode == Mode::TX)
    {
        ss << "mode: TX; msToSleep: " << t.msToSleep << "; packetSize: " << t.packetSize << "; targetaddr: " << t.targetaddr;
    }
    else
    {
        ss << "mode: RX; rcvBufSize: " << t.recvBufSize;
    }


    os << ss.str();

    return os;
}

}
