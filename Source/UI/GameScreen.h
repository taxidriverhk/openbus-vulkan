#pragma once

#include <memory>

#include <QtWidgets>

class GameScreen : public QDockWidget
{
    Q_OBJECT

public:
    GameScreen();
    ~GameScreen();

    void SetStartButtonEnabled(bool enabled);
    void SetStopButtonEnabled(bool enabled);

Q_SIGNALS:
    void AddVehicleButtonClicked();
    void StartButtonClicked();
    void StopButtonClicked();

private:
    static constexpr int BUTTON_HEIGHT = 100;

    std::unique_ptr<QWidget> buttonLayoutContainer;
    std::unique_ptr<QGridLayout> buttonLayout;

    std::unique_ptr<QPushButton> startButton;
    std::unique_ptr<QPushButton> stopButton;
    std::unique_ptr<QPushButton> addVehicleButton;
    std::unique_ptr<QPushButton> settingsButton;
};
