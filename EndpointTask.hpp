// #pragma once

// #include <variant>

// namespace ts
// {

// enum class Mode
// {
//     TX,
//     RX
// };

// struct Task //common structure to store parameter for both rx & tx
// {
//     Mode mode;

//     // TX
//     int msToSleep = 2000;
//     int sendBufferSize = 1024;
//     std::variant<sockaddr_in, sockaddr_un> targetaddr;

//     // RX
// };

// }
