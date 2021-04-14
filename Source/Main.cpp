#include <iostream>

#include "Common/Logger.h"
#include "UI/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    MainWindow mainWindow;
    mainWindow.Open();

    Logger::Log(LogLevel::Info, "Game started");
    try
    {
        return application.exec();
    }
    catch (const std::exception& ex)
    {
        Logger::Log(LogLevel::Error, ex.what());
        return EXIT_FAILURE;
    }
}
