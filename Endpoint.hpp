#pragma once

#include "SocketPair.hpp"
#include "EndpointDescription.hpp"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <variant>

namespace ts
{


class Endpoint
{

public:
    
    enum class Mode
    {
        TX,
        RX
    };

    struct Params //common structure to store parameter for both rx & tx
    {
        // TX
        int msToSleep = 2000;
        int sendBufferSize = 1024;

        // RX
    };

    void start(const SocketPair& p, Mode mode, const Params&);

    void wait() { trd.join(); }

private:

    std::thread trd;

};


class EndpointNew
{

public:

    enum class Mode
    {
        TX,
        RX
    };

    struct Task //common structure to store parameter for both rx & tx
    {
        Mode mode;

        // TX
        int msToSleep = 2000;
        int sendBufferSize = 1024;
        std::variant<sockaddr_in, sockaddr_un> targetaddr;

        // RX
    };

    EndpointNew(const EndpointDescription&);
    ~EndpointNew();

    void start();
    void waitReadtyToOperate();
    void startTask(const Task&);
    void wait() { trd.join(); }    

    std::variant<sockaddr_in, sockaddr_un> getAddr() const { return selfaddr; }

private:

    void routine();
    void init();
    void processTask();

    const EndpointDescription description;

    std::thread trd;
    std::shared_ptr<SocketWrapper> socket;
    std::variant<sockaddr_in, sockaddr_un> selfaddr;

    bool readyToOperate;
    std::optional<Task> task;

    std::mutex mutexOperational;
    std::condition_variable cvOperational;

    std::mutex mutexTask;
    std::condition_variable cvTask;

};

}