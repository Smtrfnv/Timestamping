#include "Endpoint.hpp"

#include "Sender.hpp"
#include "Receiver.hpp"
#include "Logger.hpp"
#include "util.hpp"
#include "dgram.hpp"
#include "stream.hpp"

#include <unistd.h>

namespace ts
{

#define EPLOG(...) \
    TSNAMEDLOG(description.name.c_str(), __VA_ARGS__)

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
        raiseError("Init: unsupported transport");
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
        raiseError("Failed to create socket");
    }

    socket = std::make_shared<SocketWrapper>("some name", fd, Transport::DGRAM);

    //now need to extract address and port
    if(description.selfAddr.empty() == false) //need to do bind
    {
        const auto addr = getIpV4AddressAndPort(description.selfAddr);
        if(!addr)
        {
            raiseError("Failed to create socket");
        }

        if(bind(fd, (sockaddr *)&(addr.value()), sizeof(addr.value())) != 0)
        {
            raiseError("Endpoint_udp: failed to bind");
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
    if(description.selfAddr.empty()) //this would be a client
    {
        const int clientFd = createStreamSocket();
        
        socket = std::make_shared<SocketWrapper>("Client_tcp", clientFd, Transport::STREAM);

        const auto peerAddr = getIpV4AddressAndPort(description.peerAddr);
        if(!peerAddr)
        {
            raiseError("Failed to parse peer addr");
        }
        
        if(connect(socket->getFd(), (sockaddr*) &(*peerAddr), sizeof(*peerAddr)) != 0)
        {
            raiseError("initTcp: failed to connect");
        }

        EPLOG("connectrion established");
    }
    else //server
    {
        const int listenfd = createStreamSocket();

        const auto addr = getIpV4AddressAndPort(description.selfAddr);
        if(addr.has_value() == false)
        {
            raiseError("Failed to create socket");
        }

        if(bind(listenfd, (sockaddr *) &(addr.value()), sizeof(addr.value())) != 0)
        {
            close(listenfd);
            raiseError("initTcp: failed to do bind");
        }

        if(listen(listenfd, 1) != 0)
        {
            close(listenfd);
            raiseError("initTcp: Failed to listen");
        }

        sockaddr_in cliaddr = {};
        socklen_t clilen = sizeof(clilen);

        const int serverFd = accept(listenfd, (sockaddr*) &cliaddr, &clilen);
        if(serverFd == -1)
        {
            raiseError("initTcp: failed to accept connection\n");
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
        params.sendBufferSize = task->sendBufferSize;
        params.mode = Sender::Mode::LargePackets;
        s.start(params);
    }
    else if(task->mode == Mode::RX)
    {
        EPLOG("Starting Endpoint in RX mode");

        Receiver r(socket);
        Receiver::Params params = {};
        params.bufferCapacity = 10000;
        r.start(params);
    }
    else
    {
        raiseError("Unsupported mode");
    }   
}

void EndpointNew::initDgramLocal()
{
    const int fd = createDgramLocalSocket();
    socket = std::make_shared<SocketWrapper>(description.name.c_str(), fd, Transport::DGRAM_LOCAL);

    if(description.selfAddr.empty() == false)
    {
        sockaddr_un sockaddrUn = convertUnixSocketAddr(description.selfAddr);
        
        unlink(description.selfAddr.c_str());

        int res = bind(socket->getFd(), (sockaddr*) &sockaddrUn, sizeof(sockaddr));
        if(res  != 0)
        {
            raiseError("Server_udp_local: failed to bind");
        }
        selfaddr = sockaddrUn;
    }
}

void EndpointNew::initStreamLocal()
{
    if(description.selfAddr.empty()) //this would be a client
    {
        const int fd = createStreamLocalSocket();
        
        socket = std::make_shared<SocketWrapper>(description.name.c_str(), fd, Transport::STREAM_LOCAL);

        auto peerAddr = convertUnixSocketAddr(description.peerAddr);
        
        if(connect(fd, (sockaddr*) &(peerAddr), sizeof(peerAddr)) != 0)
        {
            raiseError("initTcpLocal: failed to connect");
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
            raiseError("initTcpLocal: failed to bind");
        }

        res = listen(listenFd, 1);
        if(res != 0)
        {
            close(listenFd);
            raiseError("initTcpLocal: failed to listen");
        }

        sockaddr_un clientAddress = {};
        socklen_t clientAddrLen = sizeof(clientAddress);

        int serverFd = accept(listenFd, (sockaddr*)&clientAddress, &clientAddrLen);
        if(serverFd == -1)
        {
            raiseError("createStreamLocalPair: failed to accept connection\n");
        }

        socket = std::make_shared<SocketWrapper>(description.name.c_str(), serverFd, Transport::STREAM_LOCAL);
        selfaddr = serverAddr;
        close(listenFd);
    }
}

}
                                                                                  