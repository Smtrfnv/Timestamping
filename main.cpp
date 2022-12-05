#include <iostream>
#include <array>
#include <thread>
#include <chrono>
#include <cstring>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/net_tstamp.h>
#include <linux/errqueue.h>
#include <netinet/in.h>

namespace ts
{

const size_t MAXLEN = 100;

void raiseError( const char* x)
{
    std::cerr << x << std::endl;
    exit(1);
}

int createSocket(int family, int type, int protocol)
{
    std::cout << "creating socket\n";
    int fd = socket(family, type, protocol);
    if(fd < 0)
        raiseError("socket creation error");


    uint32_t val = 
        SOF_TIMESTAMPING_TX_HARDWARE
        | SOF_TIMESTAMPING_TX_SOFTWARE

        | SOF_TIMESTAMPING_RX_SOFTWARE
        | SOF_TIMESTAMPING_RX_HARDWARE

        | SOF_TIMESTAMPING_SOFTWARE
        | SOF_TIMESTAMPING_RAW_HARDWARE
        ;
    
    int res = setsockopt(fd, SOL_SOCKET, SO_TIMESTAMPING, &val, sizeof(val));

    if(res != 0)
    {
        raiseError("Failed to setsockopt");
    }

    return fd;
}

void processCtrlMessage(cmsghdr &chdr)
{
    std::cout << "cmsg_level = " << chdr.cmsg_level << " cmsg_type = " << chdr.cmsg_type << std::endl;
    
    switch(chdr.cmsg_level)
    {
        case(SOL_SOCKET):
        {
            switch(chdr.cmsg_type)
            {
                case(SO_TIMESTAMPING):
                {
                    std::cout << "Got SO_TIMESTAMPING\n";

                    scm_timestamping ts = {};

                    std::memcpy(&ts, CMSG_DATA(&chdr), sizeof(ts));

                    std::cout << "ts[0]" << ts.ts[0].tv_sec << " " << ts.ts[0].tv_nsec
                              << "\nts[1]" << ts.ts[1].tv_sec << " " << ts.ts[1].tv_nsec
                              << "\nts[2]" << ts.ts[2].tv_sec << " " << ts.ts[2].tv_nsec << std::endl;
                    break;
                }
                default:
                {
                    std::cout << "unsupported cmsg_type\n";
                }
            }
            break;
        }
        default:
        {
            std::cout << "Unknown cmsg_level\n";
        }
    }
}

void clientProcessingLoop(int fd, uint16_t servPort)
{
    sockaddr_in servaddr = {};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(servPort);
    int res = inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    if(res != 1)
    {
        raiseError("Failed to convert server IP address");
    }

    std::array<char, MAXLEN> sendBuf;

    std::array<char, MAXLEN * 3> iobuf;

    std::array<char, MAXLEN * 5> ctrlbuf;

    for(int val = 0; true; ++val)
    {
        const  int n = snprintf(sendBuf.begin(), sendBuf.size(), "%d", val);

        std::cout << "\nsending " << sendBuf.begin() << std::endl;
        int res = sendto(fd, sendBuf.begin(), strlen(sendBuf.begin()), 0, (sockaddr*) &servaddr, sizeof(servaddr));

        msghdr msg = {};

        iovec entry = {};
        entry.iov_base = iobuf.begin();
        entry.iov_len = iobuf.size();

        msg.msg_iov = &entry;
        msg.msg_iovlen = 1;

        // msg.msg_name = &servaddr;
        // msg.

        msg.msg_control = ctrlbuf.begin();
        msg.msg_controllen = ctrlbuf.size();

        const int rn = recvmsg(fd, &msg, MSG_ERRQUEUE);
        if(rn == -1)
        {
            std::cerr << "Failed to receive from MSG_ERRQUEUE\n";
        }
        else
        {
            std::cout << "received " << n << " bytes from MSG_ERRQUEUE\n";
            std::cout << "controllen is " << msg.msg_controllen << std::endl;

            for(cmsghdr *chdr = CMSG_FIRSTHDR(&msg); chdr != NULL; chdr = CMSG_NXTHDR(&msg, chdr))
            {
                processCtrlMessage(*chdr);
            }
        }

        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1s);
        }
    }
}

void tcpServer(const int servPort)
{   
    int listenfd = createSocket (AF_INET, SOCK_STREAM, 0);

    sockaddr_in servaddr = {};   
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
    servaddr.sin_port = htons (servPort);

    int res = bind(listenfd, (sockaddr *) &servaddr, sizeof(servaddr));
    if(res != 0)
    {
        raiseError("failed to do bind");
    }

    res = listen(listenfd, 1);
    if(res != 0)
    {
        raiseError("Failed to listen");
    }

    sockaddr_in cliaddr = {};
    socklen_t clilen = sizeof(clilen);

    int connfd = accept(listenfd, (sockaddr*) &cliaddr, &clilen);
    if(connfd == -1)
    {
        raiseError("failed to accept connection\n");
    }

    std::cout << "Server: connection accepted\n";
    
    res = close(listenfd);
    if(res != 0)
    {
        raiseError("Failed to close listenfd");
    }


    std::cout << "start sending packets\n";

    char buf[MAXLEN];
    for(int val = 0; ;++val)
    {
        snprintf(buf, MAXLEN, "%d", val);

        std::cout << "Sending " << val << std::endl;
        int n = send(connfd, buf, strlen(buf), 0);
        if(n == -1)
        {
            raiseError("Failed to send");
        }
        std::cout << "Sent " << n << " bytes\n\n\n";
        
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(2s);
        }
    }

    res = close(connfd);
    if(res != 0)
    {
        raiseError("Failed to close connfd");
    }
}


void tcpClient(int servPort)
{
    int sockfd = createSocket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in servaddr = {};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(servPort);
    int res = inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    if(res != 1)
    {
        raiseError("Failed to inet_pton");
    }
    
    res = connect(sockfd, (sockaddr*) &servaddr, sizeof(servaddr));
    if(res != 0)
    {
        raiseError("Client failed to connect");
    }

    std::cout << "Connectrion established\n";

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

    for(;;)
    {
        msghdr msg;
        msg.msg_name = 0;
        msg.msg_namelen = 0;

        msg.msg_iov = &io;
        msg.msg_iovlen = 1;

        msg.msg_control = u.buf;
        msg.msg_controllen = buffsize;

        int n = recvmsg(sockfd, &msg, 0);
        if(n == -1)
        {
            raiseError("Client failed to receive");
        }

        std::cout << "Clent received " << n << " bytes, msg_controllen is " << msg.msg_controllen << std::endl;

        buf[n] = 0;
        std::cout << "Received msg: " << buf << std::endl;

        for(cmsghdr *hdr = CMSG_FIRSTHDR(&msg); hdr != 0; hdr = CMSG_NXTHDR(&msg, hdr))
        {
            processCtrlMessage(*hdr);
        }

        std::cout << "\n";

    }
}


} //namespace ts

int main(int argc, char* argv[])
{
    // std::cout << "Hello world\n";

    // int fd = ts::createSocket(AF_INET, SOCK_DGRAM, 0);
    // ts::clientProcessingLoop(fd, 1234);

    if(argc != 2)
    {
        ts::raiseError("Number of arguments should be 2");
    }

    const int port = 24567;
    if(argv[1][0] == 'c')
    {
        ts::tcpClient(port);
    }
    else if(argv[1][0] == 's')
    {
        ts::tcpServer(port);
    }
    else
    {
        ts::raiseError("Bad argument, should be either c or s");
    }

}