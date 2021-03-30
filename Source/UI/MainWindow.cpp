#include "MainWindow.h"

MainWindow::MainWindow()
      : game(std::make_unique<Game>())
{
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    setWindowTitle(Util::FormatWindowTitle("Main Window").c_str());

    mainLayout = std::make_unique<QWidget>();

    startButton = std::make_unique<QPushButton>("Start", mainLayout.get());
    startButton->resize(100, 100);
    startButton->move(400, 200);

    shutdownButton = std::make_unique<QPushButton>("Shutdown", mainLayout.get());
    shutdownButton->setDisabled(true);
    shutdownButton->resize(100, 100);
    shutdownButton->move(400, 350);

    setCentralWidget(mainLayout.get());
    connect(startButton.get(), &QPushButton::clicked, this, &MainWindow::StartButtonClicked);
    connect(shutdownButton.get(), &QPushButton::clicked, this, &MainWindow::ShutdownButtonClicked);
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    game->SetShouldEndGame(true);
    event->accept();
}

void MainWindow::Open()
{
    show();
}

void MainWindow::ShutdownButtonClicked()
{
    game->SetShouldEndGame(true);
}

void MainWindow::StartButtonClicked()
{
    startButton->setDisabled(true);
    shutdownButton->setDisabled(false);
    game->Start();
    startButton->setDisabled(false);
    shutdownButton->setDisabled(true);
}
