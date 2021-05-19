#include "GameScreen.h"

GameScreen::GameScreen()
{
    QStyle *standardStyle = this->style();

    buttonLayoutContainer = std::make_unique<QWidget>();
    buttonLayout = std::make_unique<QGridLayout>();

    startButton = std::make_unique<QPushButton>("Start");
    startButton->setIcon(standardStyle->standardIcon(QStyle::StandardPixmap::SP_MediaPlay));
    startButton->setFixedHeight(BUTTON_HEIGHT);
    startButton->setDisabled(true);
    stopButton = std::make_unique<QPushButton>("Stop");
    stopButton->setIcon(standardStyle->standardIcon(QStyle::StandardPixmap::SP_MediaStop));
    stopButton->setFixedHeight(BUTTON_HEIGHT);
    stopButton->setDisabled(true);
    addVehicleButton = std::make_unique<QPushButton>("Add Vehicle");
    addVehicleButton->setIcon(standardStyle->standardIcon(QStyle::StandardPixmap::SP_FileDialogListView));
    addVehicleButton->setFixedHeight(BUTTON_HEIGHT);
    addVehicleButton->setDisabled(true);
    settingsButton = std::make_unique<QPushButton>("Settings");
    settingsButton->setIcon(standardStyle->standardIcon(QStyle::StandardPixmap::SP_ComputerIcon));
    settingsButton->setFixedHeight(BUTTON_HEIGHT);

    buttonLayout->addWidget(startButton.get(), 0, 0);
    buttonLayout->addWidget(stopButton.get(), 0, 1);
    buttonLayout->addWidget(addVehicleButton.get(), 0, 2);
    buttonLayout->addWidget(settingsButton.get(), 0, 3);

    setWidget(buttonLayoutContainer.get());
    buttonLayoutContainer->setLayout(buttonLayout.get());

    connect(addVehicleButton.get(), &QPushButton::clicked, this, &GameScreen::AddVehicleButtonClicked);
    connect(startButton.get(), &QPushButton::clicked, this, &GameScreen::StartButtonClicked);
    connect(stopButton.get(), &QPushButton::clicked, this, &GameScreen::StopButtonClicked);
}

GameScreen::~GameScreen()
{
}

void GameScreen::SetAddVehicleButtonEnabled(bool enabled)
{
    addVehicleButton->setEnabled(enabled);
}

void GameScreen::SetStartButtonEnabled(bool enabled)
{
    startButton->setEnabled(enabled);
}

void GameScreen::SetStopButtonEnabled(bool enabled)
{
    stopButton->setEnabled(enabled);
}
