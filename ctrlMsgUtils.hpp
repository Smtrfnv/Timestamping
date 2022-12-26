#pragma once

#include <cstdint>

#include <sys/socket.h>

#include <vector>


namespace ts
{

void processCtrlMessage(cmsghdr &chdr);
void recvCtrlMessageTx(int sockFd, uint8_t* ctrlmsgBuf, const size_t capacity);

}