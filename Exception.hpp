#pragma once

#include <exception>
#include <string>

namespace ts
{

class Exception : public std::exception
{

public:
    Exception(const char* str): error(str) {}
    Exception(const std::string& str): error(str) {}

    ~Exception() override = default;

    const char* what() const noexcept override { return error.c_str(); }

private:
    const std::string error;
};

}