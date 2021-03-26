#include "MainWindow.h"

MainWindow::MainWindow()
      : QWidget(nullptr),
        game(std::make_unique<Game>())
{
    InitComponents();
}

MainWindow::~MainWindow()
{
}

void MainWindow::InitComponents()
{
    window = std::make_unique<QWidget>();
    window->resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    window->setWindowTitle("OpenBus Vulkan|" APP_VERSION "|Startup Window");

    startButton = std::make_unique<QPushButton>("Start", window.get());
    startButton->resize(100, 100);
    startButton->move(400, 200);

    shutdownButton = std::make_unique<QPushButton>("Shutdown", window.get());
    shutdownButton->setDisabled(true);
    shutdownButton->resize(100, 100);
    shutdownButton->move(400, 350);
 
    connect(startButton.get(), &QPushButton::clicked, this, &MainWindow::StartButtonClicked);
    connect(shutdownButton.get(), &QPushButton::clicked, this, &MainWindow::ShutdownButtonClicked);
}

void MainWindow::Open()
{
    window->show();
    startButton->show();
}

void MainWindow::ShutdownButtonClicked()
{
    game->SendEndSignal();
}

void MainWindow::StartButtonClicked()
{
    startButton->setDisabled(true);
    shutdownButton->setDisabled(false);
    game->Start();
    startButton->setDisabled(false);
    shutdownButton->setDisabled(true);
}
