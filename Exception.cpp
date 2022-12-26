#include "Exception.hpp"

#include <iostream>
#include <cstring>
#include <cstdarg>

namespace ts
{

void raiseError(const char* format, ...)
{
    const size_t CAPACITY = 1024;
    char buf[CAPACITY];

    va_list args;
    va_start(args, format);

    int n = vsnprintf(buf, CAPACITY, format, args);

    va_end(args);

    snprintf(buf + n, CAPACITY - n, ", possible last error: %s", strerror(errno));

    throw Exception(buf);
}

void throwException(const char* file, int line, const char* format, ...)
{
    const size_t CAPACITY = 1024;
    char buf[CAPACITY];

    const int n1 = snprintf(buf, CAPACITY, "%s:%d ", file, line);

    va_list args;
    va_start(args, format);

    const int n2 = vsnprintf(buf + n1, CAPACITY - n1, format, args);

    va_end(args);

    snprintf(buf + n1 + n2, CAPACITY - n1 - n2, ", possible last error: %s", strerror(errno));

    throw Exception(buf);   
}

void throwNamedException(const char* file, int line, const char* name, const char* format, ...)
{
    const size_t CAPACITY = 1024;
    char buf[CAPACITY];

    const int n1 = snprintf(buf, CAPACITY, "%s:%d %s ", file, line, name);

    va_list args;
    va_start(args, format);

    const int n2 = vsnprintf(buf + n1, CAPACITY - n1, format, args);

    va_end(args);

    snprintf(buf + n1 + n2, CAPACITY - n1 - n2, ", possible last error: %s", strerror(errno));

    throw Exception(buf);       
}


}