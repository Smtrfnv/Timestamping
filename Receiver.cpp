#include "Receiver.hpp"

#include "Logger.hpp"
#include "ctrlMsgUtils.hpp"

#include <sys/uio.h>
#include <sys/socket.h>
#include <linux/errqueue.h>

namespace ts
{

Receiver::Receiver(const std::shared_ptr<SocketWrapper>& s) : socket(s)
{}

void Receiver::start(const Params& p)
{
    buffer.resize(p.bufferCapacity);

    iovec io;
    io.iov_base = buffer.data();
    io.iov_len = buffer.size() - 1;

    constexpr size_t buffsize = CMSG_SPACE(sizeof(scm_timestamping));
    union 
    {
        char buf[buffsize];
        cmsghdr chdr;
    } u;

    while(true)
    {

        msghdr msg;
        msg.msg_name = 0;
        msg.msg_namelen = 0;

        msg.msg_iov = &io;
        msg.msg_iovlen = 1;

        msg.msg_control = u.buf;
        msg.msg_controllen = buffsize;

        int n = recvmsg(socket->getFd(), &msg, 0);
        if(n == -1)
        {
            TSLOG("failed to receive");
            continue;
        }

        TSLOG("received %d bytes, msg_controllen %d", n, msg.msg_controllen);

        buffer[n] = 0;
        
        TSLOG("received msg: %s", buffer.data());

        for(cmsghdr *hdr = CMSG_FIRSTHDR(&msg); hdr != 0; hdr = CMSG_NXTHDR(&msg, hdr))
        {
            processCtrlMessage(*hdr);
        }
    }

}

}