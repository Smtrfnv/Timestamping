#pragma once



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

}