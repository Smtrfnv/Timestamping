#pragma once

#include "SocketWrapper.hpp"

#include <memory>
#include <vector>

namespace ts
{


class Receiver
{

public:

    struct Params
    {
        size_t  bufferCapacity;
    };

    Receiver(const std::shared_ptr<SocketWrapper>& s);

    void start(const Params&);

private:

    std::vector<uint8_t> buffer;

    std::shared_ptr<SocketWrapper> socket;

};

}