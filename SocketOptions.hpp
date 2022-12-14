#pragma once

#include "Exception.hpp"

#include <vector>
#include <utility>
#include <iostream>
#include <sstream>

#include <linux/net_tstamp.h>

namespace ts
{

class SocketOptions
{

public:
    SocketOptions() = default;

    bool setName(uint32_t _name)
    {
        if(name || val)
            return false;
        name = _name;
        return true;
    }

    bool set(uint32_t flag)
    {
        if(val & flag)
        {
            return false;
        }
        val |= flag;
        return true;
    }

    bool unset(uint32_t flag)
    {
        if((val & flag) == 0)
        {
            return false;
        }
        val = val & ~flag;
    }

    bool test(uint32_t flag) const
    {
        return val & flag;
    }

    uint32_t get() const
    {
        return val;
    }

    uint32_t getName() const
    {
        return name;
    } 

    friend std::ostream& operator<<(std::ostream& os, const SocketOptions& o);

    static const std::vector<std::pair<std::string, uint32_t>> optionsTable;
    static const std::vector<std::pair<std::string, uint32_t>> optionNamesTable;

private:
    uint32_t name;
    uint32_t val;
};

uint32_t stringToSockOption(const std::string& str);
uint32_t stringToSockOptionName(const std::string& str);

inline
std::ostream& operator<<(std::ostream& os, const SocketOptions& o)
{
    std::stringstream ss;

    ss << "optname: ";

    for(size_t i = 0; i < SocketOptions::optionNamesTable.size(); ++i)
    {
        const auto& elm = SocketOptions::optionNamesTable[i];
        if(o.getName() == (elm.second))
        {
            ss << elm.first;
            break;
        }
    }


    ss << ", optval: ";
    for(size_t i = 0; i < SocketOptions::optionsTable.size(); ++i)
    {
        const auto& elm = SocketOptions::optionsTable[i];
        if(o.test(elm.second))
        {
            ss << elm.first;
            if(i != SocketOptions::optionsTable.size() - 1)
            {
                ss << ", ";
            }
        }
    }

    os << ss.str();
    return os;
}


}