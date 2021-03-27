#include "UI/MainWindow.h"

#include <iostream>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    MainWindow mainWindow;
    mainWindow.Open();

    try {
        return application.exec();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
}
