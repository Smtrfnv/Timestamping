#include "Logger.hpp"

#include <cstdarg>
#include <cstddef>
#include <cstdio>

namespace ts
{

bool Logger::enabled = false;

void Logger::log(const char* file, int line, const char* format, ...)
{
    if(!enabled)
        return;

    const size_t CAPACITY = 1024;
    char buf[CAPACITY];

    va_list args;
    va_start(args, format);

    vsnprintf(buf, CAPACITY, format, args);

    va_end(args);

    printf("%s:%d, %s\n", file, line, buf);
}

void Logger::logNamed(const char* file, int line, const char* name, const char* format, ...)
{
    if(!enabled)
        return;

    const size_t CAPACITY = 1024;
    char buf[CAPACITY];

    va_list args;
    va_start(args, format);

    vsnprintf(buf, CAPACITY, format, args);

    va_end(args);

    printf("%s:%d, %s: %s\n", file, line, name, buf);
}


}