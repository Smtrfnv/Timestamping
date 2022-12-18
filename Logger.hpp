#pragma once

namespace ts
{

#define TSLOG(...) Logger::log(__FILE__, __LINE__, __VA_ARGS__);

class Logger
{
public:
    static void enable() { enabled = true; }
    static void disable() {enabled = false; }

    static void log(const char* file, int line, const char* format, ...);

private:
    static bool enabled;

};

}
