#include "Endpoint.hpp"

#include "Sender.hpp"
#include "Receiver.hpp"
#include "Logger.hpp"
#include "util.hpp"
#include "dgram.hpp"
#include "stream.hpp"

#include <unistd.h>
#include <chrono>

namespace ts
{

#define EPLOG(...) \
    TSNAMEDLOG(description.name.c_str(), __VA_ARGS__)

#define EPEXCEPTION(...) \
    THROWNAMEDEXCEPTION(description.name.c_str(), __VA_ARGS__)

EndpointNew::EndpointNew(const EndpointDescription& e): description(e)
{
    EPLOG("c-tor");
    readyToOperate = false;
}

EndpointNew::~EndpointNew()
{
    if(trd.joinable())
    {
        EPLOG("ERROR: d-to called when the thread is still joinable");
    }
    EPLOG("d-tor");
}

void EndpointNew::start()
{
    auto l = [this]() {
        this->routine();
    };
    trd = std::thread(l);
}

void EndpointNew::routine()
{
    init();

    {
        std::unique_lock<std::mutex> lock(mutexTask);
        cvTask.wait(lock, [this](){ return this->task.has_value(); });
    }
    processTask();
}

void EndpointNew::waitReadtyToOperate()
{
    std::unique_lock<std::mutex> lock(mutexOperational);
    if(readyToOperate)
        return;

    cvOperational.wait(lock, [this](){return this->readyToOperate; });
}

void EndpointNew::startTask(const Task& t)
{
    std::unique_lock<std::mutex> lock(mutexTask);

    if(task.has_value())
    {
        EPLOG("error, there is already a task");
        return;
    }

    task = t;
    cvTask.notify_one();
}

void EndpointNew::startTask()
{
    startTask(description.task);
}


void EndpointNew::init()
{ 
    std::unique_lock<std::mutex> lock(mutexOperational);

    if(description.transport == Transport::DGRAM)
    {
        initDgram();
    }
    else if(description.transport == Transport::DGRAM_LOCAL)
    {
        initDgramLocal();
    }
    else if(description.transport == Transport::STREAM)
    {
        initStream();
    }
    else if(description.transport == Transport::STREAM_LOCAL)
    {
        initStreamLocal();
    }
    else
    {
        EPEXCEPTION("unsupported transport");
    }

    readyToOperate = true;
    EPLOG("init done");
    cvOperational.notify_one();
}

void EndpointNew::initDgram()
{
    const int fd = createDgramSocket();
    if(fd == -1)
    {
        EPEXCEPTION("Failed to create socket");
    }

    if(description.sockOpts.get() && !setOptions(fd, AF_INET, description.sockOpts))
    {
        EPEXCEPTION("Failed to setOptions");
    }

    socket = std::make_shared<SocketWrapper>("some name", fd, Transport::DGRAM);

    //now need to extract address and port
    if(description.selfAddr.empty() == false) //need to do bind
    {
        const auto addr = getIpV4AddressAndPort(description.selfAddr);
        if(!addr)
        {
            EPEXCEPTION("Failed to getIpV4AddressAndPort");
        }

        if(bind(fd, (sockaddr *)&(addr.value()), sizeof(addr.value())) != 0)
        {
            EPEXCEPTION("Endpoint_udp: failed to bind");
        }
        selfaddr = *addr;
        EPLOG("binding is done")
    }
    else
    {
        EPLOG("skip binding as address is empty")
    }
}


void EndpointNew::initStream()
{
    EPLOG("init stream is called");

    if(description.selfAddr.empty()) //this would be a client
    {
        const int clientFd = createStreamSocket();
        
        socket = std::make_shared<SocketWrapper>("Client_tcp", clientFd, Transport::STREAM);

        const auto peerAddr = getIpV4AddressAndPort(description.peerAddr);
        if(!peerAddr)
        {
            EPEXCEPTION("Failed to parse peer addr");
        }

        //before connection, wait for some time to ensure that server has started
        //TODO: better to write a cycle and try to connect in a loop
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(2s);
        }
        
        if(connect(socket->getFd(), (sockaddr*) &(*peerAddr), sizeof(*peerAddr)) != 0)
        {
            EPEXCEPTION("failed to connect");
        }

        if(description.sockOpts.get() && !setOptions(clientFd, AF_INET, description.sockOpts))
        {
            EPEXCEPTION("Failed to setOptions");
        }

        EPLOG("connectrion established");
    }
    else //server
    {
        const int listenfd = createStreamSocket();

        {
            int val = 1;
            if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0)
            {
                EPEXCEPTION("Failed to set SO_REUSEADDR");
            }
        }

        const auto addr = getIpV4AddressAndPort(description.selfAddr);
        if(addr.has_value() == false)
        {
            EPEXCEPTION("Failed to create socket");
        }

        if(bind(listenfd, (sockaddr *) &(addr.value()), sizeof(addr.value())) != 0)
        {
            close(listenfd);
            EPEXCEPTION("failed to do bind");
        }

        if(listen(listenfd, 1) != 0)
        {
            close(listenfd);
            EPEXCEPTION("Failed to listen");
        }

        sockaddr_in cliaddr = {};
        socklen_t clilen = sizeof(clilen);

        const int serverFd = accept(listenfd, (sockaddr*) &cliaddr, &clilen);
        if(serverFd == -1)
        {
            EPEXCEPTION("initTcp: failed to accept connection\n");
        }

        if(description.sockOpts.get() && !setOptions(serverFd, AF_INET, description.sockOpts))
        {
            EPEXCEPTION("Failed to setOptions");
        }

        socket = std::make_shared<SocketWrapper>("Server_tcp", serverFd, Transport::STREAM);

        EPLOG("connection accepted");
    
        close(listenfd);
    }
}

void EndpointNew::processTask()
{
    if(task->mode == Mode::TX)
    {
        EPLOG("Starting Endpoint in TX mode");

        Sender s(socket);
        Sender::Params params = {};

        if(description.transport == Transport::DGRAM)
        {
            const std::optional<sockaddr_in> addr = getIpV4AddressAndPort(task->targetaddr);
            if(!addr)
            {
                throw Exception("failed to convert address when creating sender");
            }
            params.receiveraddr = *addr;
        }
        else if(description.transport == Transport::DGRAM_LOCAL)
        {
            params.receiveraddr = convertUnixSocketAddr(task->targetaddr);
        }

        params.msToSleep = task->msToSleep;
        params.sendBufferSize = task->packetSize;
        params.mode = Sender::Mode::LargePackets;
        s.start(params);
    }
    else if(task->mode == Mode::RX)
    {
        EPLOG("Starting Endpoint in RX mode");

        Receiver r(socket);
        Receiver::Params params = {};
        params.bufferCapacity = task->recvBufSize;
        r.start(params);
    }
    else
    {
        EPEXCEPTION("Unsupported mode");
    }   
}

void EndpointNew::initDgramLocal()
{
    const int fd = createDgramLocalSocket();

    if(description.sockOpts.get() && !setOptions(fd, AF_UNIX, description.sockOpts))
    {
        EPEXCEPTION("Failed to setOptions");
    }

    socket = std::make_shared<SocketWrapper>(description.name.c_str(), fd, Transport::DGRAM_LOCAL);

    if(description.selfAddr.empty() == false)
    {
        sockaddr_un sockaddrUn = convertUnixSocketAddr(description.selfAddr);
        
        unlink(description.selfAddr.c_str());

        int res = bind(socket->getFd(), (sockaddr*) &sockaddrUn, sizeof(sockaddr));
        if(res  != 0)
        {
            EPEXCEPTION("Server_udp_local: failed to bind");
        }
        selfaddr = sockaddrUn;
    }
}

void EndpointNew::initStreamLocal()
{
    if(description.selfAddr.empty()) //this would be a client
    {
        const int fd = createStreamLocalSocket();

        if(description.sockOpts.get() && !setOptions(fd, AF_UNIX, description.sockOpts))
        {
            EPEXCEPTION("Failed to setOptions");
        }
        
        socket = std::make_shared<SocketWrapper>(description.name.c_str(), fd, Transport::STREAM_LOCAL);

        auto peerAddr = convertUnixSocketAddr(description.peerAddr);

        //before connection, wait for some time to ensure that server has started
        //TODO: better to write a cycle and try to connect in a loop
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(2s);
        }
        
        if(connect(fd, (sockaddr*) &(peerAddr), sizeof(peerAddr)) != 0)
        {
            EPEXCEPTION("initTcpLocal: failed to connect");
        }

        EPLOG("connectrion established");
    }    
    else // server
    {
        const int listenFd = createStreamLocalSocket();

        sockaddr_un serverAddr = convertUnixSocketAddr(description.selfAddr);

        unlink(description.selfAddr.c_str());

        int res = bind(listenFd, (sockaddr*) &serverAddr, sizeof(serverAddr));
        if(res != 0)
        {
            close(listenFd);
            EPEXCEPTION("initTcpLocal: failed to bind");
        }

        res = listen(listenFd, 1);
        if(res != 0)
        {
            close(listenFd);
            EPEXCEPTION("initTcpLocal: failed to listen");
        }

        sockaddr_un clientAddress = {};
        socklen_t clientAddrLen = sizeof(clientAddress);

        int serverFd = accept(listenFd, (sockaddr*)&clientAddress, &clientAddrLen);
        if(serverFd == -1)
        {
            EPEXCEPTION("failed to accept connection\n");
        }

        if(description.sockOpts.get() && !setOptions(serverFd, AF_UNIX, description.sockOpts))
        {
            EPEXCEPTION("Failed to setOptions");
        }

        socket = std::make_shared<SocketWrapper>(description.name.c_str(), serverFd, Transport::STREAM_LOCAL);
        selfaddr = serverAddr;
        close(listenFd);
    }
}

}
                                                                                  