cmake_minimum_required(VERSION 3.20)
project(timestamping)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_library(app 
    stream.cpp
    dgram.cpp 
    util.cpp 
    SocketWrapper.cpp 
    Logger.cpp 
    Sender.cpp 
    Receiver.cpp 
    ctrlMsgUtils.cpp 
    Endpoint.cpp
    EndpointFactory.cpp
    ConfigParser.cpp)

target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../rapidjson/include/) #TODO: replace hardcoded path, also: hide dependancy

add_executable(ts main.cpp)
target_link_libraries(ts app)
