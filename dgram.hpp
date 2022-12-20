#pragma once

#include "util.hpp"

namespace ts
{

SocketPair createUdpPair();

SocketPair createDgramLocalPair();

int createDgramSocket();
// returns -1 in case of a failure, and socket handle otherwise

}
