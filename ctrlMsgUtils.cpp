#include "ctrlMsgUtils.hpp"

#include "Logger.hpp"
#include "util.hpp"

#include <cstring>

#include <linux/errqueue.h>
#include <netinet/in.h>

namespace ts
{

void processCtrlMessage(cmsghdr &chdr)
{
    TSLOG("cmsg_level = %d, cmsg_type = %d, cmsg_payload len = %d", chdr.cmsg_level, chdr.cmsg_type, chdr.cmsg_len - sizeof(chdr));
    
    switch(chdr.cmsg_level)
    {
        case(SOL_SOCKET):
        {
            switch(chdr.cmsg_type)
            {
                case(SO_TIMESTAMPING):
                {
                    TSLOG("Got SO_TIMESTAMPING");

                    scm_timestamping ts = {};

                    std::memcpy(&ts, CMSG_DATA(&chdr), sizeof(ts));

                    TSLOG("ts[0] %ld %ld", ts.ts[0].tv_sec, ts.ts[0].tv_nsec);
                    TSLOG("ts[1] %ld %ld", ts.ts[1].tv_sec, ts.ts[1].tv_nsec);
                    TSLOG("ts[2] %ld %ld", ts.ts[2].tv_sec, ts.ts[2].tv_nsec);
                    break;
                }
                case(SO_TIMESTAMPNS):
                {
                    TSLOG("Got SO_TIMESTAMPNS");

                    timespec ts = {};
                    std::memcpy(&ts, CMSG_DATA(&chdr), sizeof(ts));
    
                    TSLOG("ts: %ld %ld", ts.tv_sec, ts.tv_nsec);
                    break;
                }
                default:
                {
                    TSLOG("unsupported cmsg_type");
                }
            } break;
        }
// IPPROTO_IP
        case (SOL_IP):
        {
            TSLOG("Got SOL_IP");

            switch(chdr.cmsg_type)
            {
                case(IP_RECVERR):
                {
                    TSLOG("Got IP_RECVERR");

                    sock_extended_err ser = {};
                    std::memcpy(&ser, CMSG_DATA(&chdr), sizeof(ser));

                    TSLOG("errno=%u, strerr=%s, origin=%u, type=%u, code=%u, pad=%u, info=%u, data=%u",
                            ser.ee_errno, strerror(ser.ee_errno), ser.ee_origin, ser.ee_type, ser.ee_code, ser.ee_pad, ser.ee_info, ser.ee_data);

                } break;

                default:
                {
                    TSLOG("Unsupported cmsg_type");
                }
            }

        } break;

        default:
        {
            TSLOG("Unknown cmsg_level");
        }
    }
}

void recvCtrlMessageTx(int sockFd)
{
    recvCtrlMessageTx(sockFd, nullptr, 0);
}

void recvCtrlMessageTx(int sockFd, uint8_t* buf, size_t capacity)
{
    iovec io;
    io.iov_base = buf;
    io.iov_len = capacity;

    constexpr size_t buffsize = CMSG_SPACE(sizeof(scm_timestamping) * 6); //TODO
    union 
    {
        char ctrlBuf[buffsize];
        cmsghdr chdr;
    } u;

    msghdr msg = {};
    msg.msg_name = 0;
    msg.msg_namelen = 0;

    msg.msg_iov = /*&io*/ 0;
    msg.msg_iovlen = /*1*/ 0;

    msg.msg_control = u.ctrlBuf;
    msg.msg_controllen = buffsize;

    int res = recvmsg(sockFd, &msg, MSG_ERRQUEUE);
    
    TSLOG("recvmsg result: %d, flags = %d, iov_len is %lu", res, msg.msg_flags, io.iov_len);

    // for(int i = 0; i < 10; ++i)
    // {
    //     std::cout << (int32_t)(buf[i]);
    // }
    // std::cout << std::endl;

    if(res == -1)
    {
        raiseError("recvmsg error");
    }

    if(res != -1)
    {
        for(cmsghdr *hdr = CMSG_FIRSTHDR(&msg); hdr != 0; hdr = CMSG_NXTHDR(&msg, hdr))
        {
            processCtrlMessage(*hdr);
        }
    }
}

}