#pragma once

#include <exception>
#include <string>

#define THROWEXCEPTION(...) throwException(__FILE__, __LINE__, __VA_ARGS__);
#define THROWNAMEDEXCEPTION(name, ...) throwNamedException(__FILE__, __LINE__, name, __VA_ARGS__);

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

void raiseError(const char* format, ...);
void throwException(const char* file, int line, const char* format, ...);
void throwNamedException(const char* file, int line, const char* name, const char* format, ...);

}