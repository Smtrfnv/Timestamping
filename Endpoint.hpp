#pragma once

#include "EndpointDescription.hpp"

#include "SocketWrapper.hpp"
#include "EndpointTask.hpp"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <variant>

#include <sys/un.h>
#include <netinet/in.h>

namespace ts
{

class EndpointNew
{

public:

    EndpointNew(const EndpointDescription&);
    ~EndpointNew();

    void start();
    void waitReadtyToOperate();
    void startTask(const Task&);
    void startTask();
    void wait() { trd.join(); }    

    std::variant<sockaddr_in, sockaddr_un> getAddr() const { return selfaddr; }

private:

    void routine();
    void init();
    void initDgram();
    void initDgramLocal();
    void initStream();
    void initStreamLocal();

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