#include "Common/Logger.h"
#include "UI/MessageDialog.h"
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
        MessageDialog errorMessageBox;
        errorMessageBox.ShowMessage(
            MessageDialogType::Fatal,
            "The program encounters a fatal error and must exit, check the error below for details.",
            ex.what());
        return EXIT_FAILURE;
    }
}
