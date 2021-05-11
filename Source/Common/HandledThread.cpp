#include "Logger.h"

#include "HandledThread.h"

HandledThread::HandledThread(std::function<void()> func, std::function<void()> signalFunc)
{
    thread = std::thread([&, func, signalFunc]()
        {
            try
            {
                func();
            }
            catch (std::exception &ex)
            {
                Logger::Log(LogLevel::Error, ex.what());
                lastException = std::current_exception();
                // This signal function should only be used to signal the GUI thread
                // to handle failure, the body of the signal function itself must not deal with the GUI components
                // as this signal function is called within this thread
                signalFunc();
            }
        });
}

void HandledThread::Join()
{
    if (thread.joinable())
    {
        thread.join();
    }
}
