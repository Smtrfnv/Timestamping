#include <iostream>
#include <array>
#include <thread>
#include <chrono>
#include <cstring>
#include <unistd.h>

#include <linux/errqueue.h>
#include <netinet/in.h>

#include "stream.hpp"
#include "dgram.hpp"
#include "util.hpp"

namespace ts
{

const size_t MAXLEN = 100;


SocketPair createSocketPair(Transport t)
{
    SocketPair p;
    if(t == Transport::TCP)
    {
        p = createTcpPair();
        std::cout << "TCP socket pair created\n";
    }
    else if(t == Transport::UDP)
    {
        p = createUdpPair();
        std::cout << "UDP socket pair created\n";
    }
    else
        raiseError("Unsupported transport type");
    return p;
}

void doReceive(int fd)
{
    char buf[MAXLEN];
    iovec io;
    io.iov_base = buf;
    io.iov_len = MAXLEN - 1;

    constexpr size_t buffsize = CMSG_SPACE(sizeof(scm_timestamping));
    union 
    {
        char buf[buffsize];
        cmsghdr chdr;
    } u;

    msghdr msg;
    msg.msg_name = 0;
    msg.msg_namelen = 0;

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;

    msg.msg_control = u.buf;
    msg.msg_controllen = buffsize;

    int n = recvmsg(fd, &msg, 0);
    if(n == -1)
    {
        raiseError("Server: failed to receive");
    }

    std::cout << "Server: received " << n << " bytes, msg_controllen is " << msg.msg_controllen << std::endl;

    buf[n] = 0;
    std::cout << "Server: received msg: " << buf << std::endl;

    for(cmsghdr *hdr = CMSG_FIRSTHDR(&msg); hdr != 0; hdr = CMSG_NXTHDR(&msg, hdr))
    {
        processCtrlMessage(*hdr);
    }

    std::cout << "\n";
}

void doSend(Transport t, int fd, const char *buf, const sockaddr_in& addr)
{
    std::cout << "Client: sending " << buf << std::endl;

    int n = 0;
    if(t == Transport::TCP)
    {
        n = send(fd, buf, strlen(buf), 0);
    }
    else
    {
        n = sendto(fd, &buf, strlen(buf), 0, (sockaddr*) &addr, sizeof(addr));
    }

    if(n == -1)
    {
        raiseError("Failed to send");
    }
    std::cout << "Client: sent " << n << " bytes\n\n\n";
}

} //namespace ts

int main(int argc, char* argv[])
{
    using namespace ts;

    const Transport transport = Transport::UDP;

    const SocketPair p = createSocketPair(transport);

    for(int i = 0; i < 10; ++i)
    {
        {
            char buf[MAXLEN];
            snprintf(buf, MAXLEN, "%d", i);
            
            doSend(transport, p.clientFd, buf, p.serveraddr);
        }
        doReceive(p.serverFd);

        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(2s);
        }
    }


}