#pragma once
#include <string>

namespace transity::logging {

class ILogSink {
public:
    virtual void write(const std::string& message) = 0;
    virtual void flush() = 0;
    virtual ~ILogSink() = default;
};

}