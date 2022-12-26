#include "SocketOptions.hpp"

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

    
    throw Exception("Failed to convert string to transport");    
}

}