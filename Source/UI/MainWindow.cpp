#include "LogViewer.h"
#include "MainWindow.h"

MainWindow::MainWindow()
{
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    setWindowTitle(Util::FormatWindowTitle("Main Window").c_str());

    mainLayout = std::make_unique<QMainWindow>();

    startAction = std::make_unique<QAction>("Start", this);
    shutdownAction = std::make_unique<QAction>("Shutdown", this);
    shutdownAction->setEnabled(false);
    exitAction = std::make_unique<QAction>("Exit", this);

    gameMenu = menuBar()->addMenu("Game");
    gameMenu->addAction(startAction.get());
    gameMenu->addAction(shutdownAction.get());
    gameMenu->addSeparator();
    gameMenu->addAction(exitAction.get());

    setCentralWidget(mainLayout.get());

    gameScreen = std::make_unique<QDockWidget>();
    gameScreen->setFeatures(QDockWidget::DockWidgetFeature::NoDockWidgetFeatures);
    mainLayout->addDockWidget(Qt::TopDockWidgetArea, gameScreen.get());

    logViewer = std::make_unique<LogViewer>();
    logViewer->setFeatures(QDockWidget::DockWidgetFeature::NoDockWidgetFeatures);
    mainLayout->addDockWidget(Qt::BottomDockWidgetArea, logViewer.get());

    connect(exitAction.get(), &QAction::triggered, this, &MainWindow::ExitButtonClicked);
    connect(startAction.get(), &QAction::triggered, this, &MainWindow::StartButtonClicked);
    connect(shutdownAction.get(), &QAction::triggered, this, &MainWindow::ShutdownButtonClicked);
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    EndGame();
    event->accept();
}

void MainWindow::Open()
{
    show();
}

void MainWindow::EndGame()
{
    if (game != nullptr)
    {
        game->SetShouldEndGame(true);
        gameThread->join();
    }

    startAction->setDisabled(false);
    shutdownAction->setDisabled(true);
}

void MainWindow::ExitButtonClicked()
{
    close();
}

void MainWindow::ShutdownButtonClicked()
{
    EndGame();
}

void MainWindow::StartButtonClicked()
{
    startAction->setDisabled(true);
    shutdownAction->setDisabled(false);
    
    gameThread = std::make_unique<std::thread>([&]
        {
            game = std::make_unique<Game>();
            game->Start();
        });
}
