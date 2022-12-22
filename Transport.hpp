#pragma once

#include "Exception.hpp"

#include<string>

namespace ts
{

enum class Transport
{
    STREAM,
    DGRAM,
    DGRAM_LOCAL,
    STREAM_LOCAL
};

inline const char* toString(Transport t)
{
    switch(t)
    {
        case(Transport::STREAM): return "STREAM";
        case(Transport::DGRAM): return "DGRAM";
        case(Transport::DGRAM_LOCAL): return "DGRAM_LOCAL";
        case(Transport::STREAM_LOCAL): return "STREAM_LOCAL";
    }
    return "undefined transport";
}

inline
Transport stringToTransport(const std::string& str)
{
    auto eq = [](auto a, auto b) { return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                                                        [](char a, char b) { return tolower(a) == tolower(b); }
                                                    );
                                 };

    const std::string stream = "stream";
    const std::string dgram = "dgram";
    const std::string dgram_local = "dgram_local";
    const std::string stream_local = "stream_local";

    if(eq(stream, str))
        return Transport::STREAM;
    
    if(eq(stream_local, str))
        return Transport::STREAM_LOCAL;

    if(eq(dgram, str))
        return Transport::DGRAM;

    if(eq(dgram_local, str))
        return Transport::DGRAM_LOCAL;
    
    throw Exception("Failed to convert string to transport");
}



}