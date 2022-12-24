#pragma once

#include "Exception.hpp"

#include <string>

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
    int sendBufferSize = 1024;
    std::string targetaddr; //only for connectionless protocols

    // RX
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

}
