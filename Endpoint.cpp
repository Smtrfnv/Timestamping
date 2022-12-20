#include "Endpoint.hpp"

#include "Sender.hpp"
#include "Receiver.hpp"
#include "Logger.hpp"
#include "util.hpp"
#include "dgram.hpp"

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

            EPLOG("addr is %u, port is %u", addr->sin_addr.s_addr, addr->sin_port);

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
    else
    {
        raiseError("Unsupported transport");
    }

    readyToOperate = true;
    cvOperational.notify_one();
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

}
                                                                                  