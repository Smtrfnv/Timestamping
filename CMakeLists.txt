cmake_minimum_required(VERSION 3.20)
project(timestamping)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_library(app

    ConfigParser.cpp
    ctrlMsgUtils.cpp
    dgram.cpp
    Endpoint.cpp
    EndpointFactory.cpp
    Exception.cpp
    Logger.cpp 
    Receiver.cpp 
    Sender.cpp  
    SocketOptions.cpp
    SocketWrapper.cpp 
    stream.cpp
    util.cpp 
    )

target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../rapidjson/include/) #TODO: replace hardcoded path, also: hide dependancy

add_executable(ts main.cpp)
target_link_libraries(ts app)
