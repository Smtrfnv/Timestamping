#include "SocketOptions.hpp"

#include <asm-generic/socket.h>

namespace ts
{


const std::vector<std::pair<std::string, uint32_t>> SocketOptions::optionsTable = {
        {"SOF_TIMESTAMPING_TX_SOFTWARE", SOF_TIMESTAMPING_TX_SOFTWARE},
        {"SOF_TIMESTAMPING_TX_HARDWARE", SOF_TIMESTAMPING_TX_HARDWARE},
        {"SOF_TIMESTAMPING_RX_SOFTWARE", SOF_TIMESTAMPING_RX_SOFTWARE},
        {"SOF_TIMESTAMPING_RX_HARDWARE", SOF_TIMESTAMPING_RX_HARDWARE},
        {"SOF_TIMESTAMPING_TX_SCHED", SOF_TIMESTAMPING_TX_SCHED},
        {"SOF_TIMESTAMPING_TX_ACK", SOF_TIMESTAMPING_TX_ACK},

        {"SOF_TIMESTAMPING_SOFTWARE", SOF_TIMESTAMPING_SOFTWARE},
        {"SOF_TIMESTAMPING_RAW_HARDWARE", SOF_TIMESTAMPING_RAW_HARDWARE},

        {"SOF_TIMESTAMPING_OPT_ID", SOF_TIMESTAMPING_OPT_ID},
        {"SOF_TIMESTAMPING_OPT_TSONLY", SOF_TIMESTAMPING_OPT_TSONLY},
        {"SOF_TIMESTAMPING_OPT_STATS", SOF_TIMESTAMPING_OPT_STATS}
    };

const std::vector<std::pair<std::string, uint32_t>> SocketOptions::optionNamesTable = {
        {"SO_TIMESTAMPING", SO_TIMESTAMPING},
        {"SO_TIMESTAMPNS", SO_TIMESTAMPNS}
    };

uint32_t stringToSockOption(const std::string& str)
{
    auto eq = [](auto a, auto b) { return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                                                        [](char a, char b) { return tolower(a) == tolower(b); }
                                                    );
                                 };

    for(const auto& elm: SocketOptions::optionsTable)
    {
        if(eq(elm.first, str))
            return elm.second;
    }

    
    THROWEXCEPTION("Failed to convert string to optionValue");    
    return 0;
}

uint32_t stringToSockOptionName(const std::string& str)
{
    auto eq = [](auto a, auto b) { return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                                                        [](char a, char b) { return tolower(a) == tolower(b); }
                                                    );
                                 };

    for(const auto& elm: SocketOptions::optionNamesTable)
    {
        if(eq(elm.first, str))
            return elm.second;
    }

    
    THROWEXCEPTION("Failed to convert string to optionName");   
    return 0; 
}

}