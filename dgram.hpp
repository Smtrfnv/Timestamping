#pragma once

#include "util.hpp"

namespace ts
{

int createDgramSocket();
// returns -1 in case of a failure, and socket handle otherwise

int createDgramLocalSocket();

}
