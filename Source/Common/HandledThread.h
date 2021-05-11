#pragma once

#include <functional>
#include <stdexcept>
#include <thread>

class HandledThread
{
public:
    HandledThread(std::function<void()> func, std::function<void()> signalFunc);

    void Join();

private:
    std::exception_ptr lastException;
    std::thread thread;
};
