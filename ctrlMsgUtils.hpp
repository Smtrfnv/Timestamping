#pragma once

#include <cstdint>

#include <sys/socket.h>


namespace ts
{

void processCtrlMessage(cmsghdr &chdr);
void recvCtrlMessageTx(int sockFd, uint8_t* buf, size_t capacity);
void recvCtrlMessageTx(int sockFd);

}