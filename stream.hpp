#pragma once

#include "util.hpp"

namespace ts
{

SocketPair createTcpPair();

SocketPair createStreamLocalPair();

int createStreamSocket();

}