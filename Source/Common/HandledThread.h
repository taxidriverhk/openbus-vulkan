#pragma once

#include <stdexcept>
#include <thread>

#include "Logger.h"

class HandledThread
{
public:
    template<class Func, class SignalFunc, class... FuncArgs>
    HandledThread(Func &&func, SignalFunc &&signalFunc, FuncArgs && ...args)
    {
        thread = std::thread([&](Func f, FuncArgs ...a)
            {
                try
                {
                    f(a...);
                }
                catch (const std::exception &ex)
                {
                    Logger::Log(LogLevel::Error, ex.what());
                    lastException = std::current_exception();
                    // This signal function should only be used to signal the GUI thread
                    // to handle failure, the body of the signal function itself must not deal with the GUI components
                    // as this signal function is called within this thread
                    signalFunc();
                }
            },
            func,
            args...);
    }

    void Join()
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

private:
    std::exception_ptr lastException;
    std::thread thread;
};
