#pragma once

#include <memory>
#include <string>
#include <vector>

#include <QtWidgets>

class VehicleSelectionScreen : public QDockWidget
{
    Q_OBJECT

public:
    VehicleSelectionScreen();
    ~VehicleSelectionScreen();

    std::string GetSelectedVehicleFilePath() const;

Q_SIGNALS:
    void ConfirmButtonClicked();
    void CancelButtonClicked();

private:
    static constexpr int IMAGE_MIN_WIDTH = 350;
    static constexpr int IMAGE_MIN_HEIGHT = 200;

    void SelectedVehicleChanged(const int index);
    void LoadVehicles();

    std::unique_ptr<QWidget> layoutContainer;
    std::unique_ptr<QGridLayout> layout;

    std::unique_ptr<QLabel> currentVehicleImageContainer;
    std::unique_ptr<QComboBox> vehicleDropdown;

    std::unique_ptr<QBoxLayout> buttonLayout;
    std::unique_ptr<QPushButton> confirmButton;
    std::unique_ptr<QPushButton> cancelButton;

    std::string currentSelectedPath;
    std::vector<std::string> vehicleConfigFilePaths;
};
