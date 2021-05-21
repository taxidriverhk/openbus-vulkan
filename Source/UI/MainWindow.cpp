#include "Common/HandledThread.h"
#include "Config/GameConfig.h"
#include "Game/Object/GameObjectLoader.h"
#include "GameScreen.h"
#include "MapList.h"
#include "MessageDialog.h"
#include "LogViewer.h"
#include "VehicleSelectionScreen.h"
#include "MainWindow.h"

MainWindow::MainWindow()
{
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    setWindowTitle(Util::FormatWindowTitle("Main Window").c_str());

    mainLayout = std::make_unique<QWidget>();
    gridLayout = std::make_unique<QGridLayout>(this);

    mainLayout->setLayout(gridLayout.get());

    exitAction = std::make_unique<QAction>("Exit", this);

    gameMenu = menuBar()->addMenu("Game");
    gameMenu->addSeparator();
    gameMenu->addAction(exitAction.get());

    setCentralWidget(mainLayout.get());

    gameScreen = std::make_unique<GameScreen>();
    gameScreen->setFeatures(QDockWidget::DockWidgetFeature::NoDockWidgetFeatures);
    gridLayout->setRowStretch(0, 15);
    gridLayout->addWidget(gameScreen.get(), 0, 0);

    mapList = std::make_unique<MapList>();
    mapList->setFeatures(QDockWidget::DockWidgetFeature::NoDockWidgetFeatures);
    gridLayout->setRowStretch(1, 50);
    gridLayout->addWidget(mapList.get(), 1, 0);

    logViewer = std::make_unique<LogViewer>();
    logViewer->setFeatures(QDockWidget::DockWidgetFeature::NoDockWidgetFeatures);
    gridLayout->setRowStretch(2, 35);
    gridLayout->addWidget(logViewer.get(), 2, 0);

    vehicleSelectionScreen = std::make_unique<VehicleSelectionScreen>();
    vehicleSelectionScreen->setFeatures(QDockWidget::DockWidgetFeature::NoDockWidgetFeatures);

    connect(gameScreen.get(), &GameScreen::AddVehicleButtonClicked, this, &MainWindow::AddVehicleButtonClicked);
    connect(gameScreen.get(), &GameScreen::StartButtonClicked, this, &MainWindow::StartButtonClicked);
    connect(gameScreen.get(), &GameScreen::StopButtonClicked, this, &MainWindow::ShutdownButtonClicked);

    connect(vehicleSelectionScreen.get(), &VehicleSelectionScreen::ConfirmButtonClicked, this, &MainWindow::AddVehicleConfirmed);
    connect(vehicleSelectionScreen.get(), &VehicleSelectionScreen::CancelButtonClicked, this, &MainWindow::AddVehicleCanceled);

    connect(mapList.get(), &MapList::MapListItemSelected, this, &MainWindow::ResetButtonState);
    connect(exitAction.get(), &QAction::triggered, this, &MainWindow::ExitButtonClicked);
    connect(this, &MainWindow::GameThreadError, this, &MainWindow::EndGame);
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
        gameThread->Join();
    }

    ResetButtonState();
}

void MainWindow::ResetButtonState()
{
    gameScreen->SetStartButtonEnabled(true);
    gameScreen->SetStopButtonEnabled(false);
    gameScreen->SetAddVehicleButtonEnabled(false);
}

void MainWindow::AddVehicleButtonClicked()
{
    mapList->hide();
    vehicleSelectionScreen->show();
    gameScreen->SetAddVehicleButtonEnabled(false);
    gridLayout->removeWidget(mapList.get());
    gridLayout->addWidget(vehicleSelectionScreen.get(), 1, 0);
}

void MainWindow::AddVehicleConfirmed()
{
    std::string configFilePath = vehicleSelectionScreen->GetSelectedVehicleFilePath();

    GameObjectLoadRequest request{};
    request.configFilePath = configFilePath;
    request.type = GameObjectType::Vehicle;
    request.isUserObject = true;

    game->AddUserGameObject(request);
    // Close the vehicle selection screen
    AddVehicleCanceled();
}

void MainWindow::AddVehicleCanceled()
{
    vehicleSelectionScreen->hide();
    mapList->show();
    gameScreen->SetAddVehicleButtonEnabled(true);
    gridLayout->removeWidget(vehicleSelectionScreen.get());
    gridLayout->addWidget(mapList.get(), 1, 0);
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
    gameScreen->SetStartButtonEnabled(false);
    gameScreen->SetStopButtonEnabled(true);
    gameScreen->SetAddVehicleButtonEnabled(true);
    
    std::string mapPath;
    if (mapList->GetSelectedMapFile(mapPath))
    {
        startConfig.mapConfigPath = mapPath;
        startConfig.enableFrameRateDisplay = false;

        game = std::make_unique<Game>();
        gameThread = std::make_unique<HandledThread>([&]()
            {
                game->Start(startConfig);
            },
            [&]()
            {
                // Terminate the game loop thread and clean up graphics context
                game->Cleanup();
                GameThreadError();
            });
    }
    else
    {
        MessageDialog mapNotFoundDialog;
        mapNotFoundDialog.ShowMessage(MessageDialogType::Alert, "Unable to find the map file");
        EndGame();
    }
}
