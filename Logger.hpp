#pragma once

namespace ts
{

#define TSLOG(...) Logger::log(__FILE__, __LINE__, __VA_ARGS__);
#define TSNAMEDLOG(name, ...) Logger::logNamed(__FILE__, __LINE__, name, __VA_ARGS__);

class Logger
{
public:

    Logger() = delete;

    static void enable() { enabled = true; }
    static void disable() {enabled = false; }

    static void log(const char* file, int line, const char* format, ...);
    static void logNamed(const char* file, int line, const char* name, const char* format, ...);

private:
    static bool enabled;

};

}
