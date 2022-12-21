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

void Endpoint::start(const SocketPair& p, Mode mode, const Params& par)
{
    if(mode == Mode::TX)
    {
        TSLOG("Starting Endpoint in TX mode");

        auto l = [&](){

            pthread_setname_np(pthread_self(), "TX thread"); 

            Sender s(p.clientFd);
            Sender::Params params = {};
            params.receiveraddr = p.serveraddr;
            params.msToSleep = par.msToSleep;
            params.sendBufferSize = par.sendBufferSize;
            params.mode = Sender::Mode::LargePackets;
            s.start(params);
        };
        trd = std::thread(l);
    }
    else if(mode == Mode::RX)
    {
        TSLOG("Starting Endpoint in RX mode");

        auto l = [&](){

            pthread_setname_np(pthread_self(), "RX thread"); 

            Receiver r(p.serverFd);
            Receiver::Params params = {};
            params.bufferCapacity = 10000;
            r.start(params);
        };
        trd = std::thread(l);
    }
}


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

void EndpointNew::init()
{ 
    std::unique_lock<std::mutex> lock(mutexOperational);

    if(description.transport == Transport::UDP)
    {
        initUdp();
    }
    if(description.transport == Transport::UDP_LOCAL)
    {
        initUdpLocal();
    }
    else if(description.transport == Transport::TCP)
    {
        initTcp();
    }
    else
    {
        raiseError("Unsupported transport");
    }

    readyToOperate = true;
    EPLOG("init done");
    cvOperational.notify_one();
}

void EndpointNew::initUdp()
{
    const int fd = createDgramSocket();
    if(fd == -1)
    {
        raiseError("Failed to create socket");
    }

    socket = std::make_shared<SocketWrapper>("some name", fd, Transport::UDP);

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


void EndpointNew::initTcp()
{
    if(description.selfAddr.empty()) //this would be a client
    {
        const int clientFd = createStreamSocket();
        
        socket = std::make_shared<SocketWrapper>("Client_tcp", clientFd, Transport::TCP);

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
        socket = std::make_shared<SocketWrapper>("Server_tcp", serverFd, Transport::TCP);

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
        params.receiveraddr = task->targetaddr;
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

void EndpointNew::initUdpLocal()
{
    const int fd = createDgramLocalSocket();
    socket = std::make_shared<SocketWrapper>(description.name.c_str(), fd, Transport::UDP_LOCAL);

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

}
                                                                                  