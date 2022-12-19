#pragma once



namespace ts
{

enum class Transport
{
    TCP,
    UDP,
    UDP_LOCAL,
    TCP_LOCAL
};

inline const char* toString(Transport t)
{
    switch(t)
    {
        case(Transport::TCP): return "TCP";
        case(Transport::UDP): return "UDP";
        case(Transport::UDP_LOCAL): return "UDP_LOCAL";
        case(Transport::TCP_LOCAL): return "TCP_LOCAL";
    }
    return "undefined transport";
}

}