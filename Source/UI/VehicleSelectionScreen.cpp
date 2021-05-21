#include <filesystem>

#include "Common/FileSystem.h"
#include "Config/ConfigReader.h"
#include "Config/GameObjectConfig.h"
#include "VehicleSelectionScreen.h"

VehicleSelectionScreen::VehicleSelectionScreen()
{
    layoutContainer = std::make_unique<QWidget>();
    layout = std::make_unique<QGridLayout>();

    vehicleDropdown = std::make_unique<QComboBox>();

    QPixmap defaultImage(IMAGE_MIN_WIDTH, IMAGE_MIN_HEIGHT);
    defaultImage.fill(Qt::black);
    currentVehicleImageContainer = std::make_unique<QLabel>();
    currentVehicleImageContainer->resize(IMAGE_MIN_WIDTH, IMAGE_MIN_HEIGHT);
    currentVehicleImageContainer->setPixmap(defaultImage);

    buttonLayout = std::make_unique<QBoxLayout>(QBoxLayout::Direction::LeftToRight);
    confirmButton = std::make_unique<QPushButton>("Confirm");
    confirmButton->setEnabled(false);
    cancelButton = std::make_unique<QPushButton>("Cancel");

    buttonLayout->addWidget(confirmButton.get());
    buttonLayout->addWidget(cancelButton.get());

    layout->addWidget(currentVehicleImageContainer.get(), 0, 0, Qt::AlignmentFlag::AlignCenter);
    layout->addWidget(vehicleDropdown.get(), 0, 1);
    layout->addLayout(buttonLayout.get(), 1, 1);

    setWidget(layoutContainer.get());
    layoutContainer->setLayout(layout.get());

    connect(
        vehicleDropdown.get(),
        static_cast<void (QComboBox:: *)(const int)>(&QComboBox::currentIndexChanged),
        this,
        &VehicleSelectionScreen::SelectedVehicleChanged);
    connect(confirmButton.get(), &QPushButton::clicked, this, &VehicleSelectionScreen::ConfirmButtonClicked);
    connect(cancelButton.get(), &QPushButton::clicked, this, &VehicleSelectionScreen::CancelButtonClicked);

    LoadVehicles();
}

VehicleSelectionScreen::~VehicleSelectionScreen()
{
}

std::string VehicleSelectionScreen::GetSelectedVehicleFilePath() const
{
    return currentSelectedPath;
}

void VehicleSelectionScreen::LoadVehicles()
{
    std::string vehicleDirectory = FileSystem::VehicleDirectory();
    for (const auto &vehicleEntry : std::filesystem::directory_iterator(vehicleDirectory))
    {
        if (vehicleEntry.is_directory())
        {
            // Add vehicle to the list only if the directory contains vehicle.json file
            for (const auto &vehicleFile : std::filesystem::directory_iterator(vehicleEntry))
            {
                if (vehicleFile.is_regular_file() && vehicleFile.path().filename() == FileSystem::VEHICLE_FILE_NAME)
                {
                    VehicleConfig vehicleConfig;
                    if (ConfigReader::ReadConfig(vehicleFile.path().string(), vehicleConfig))
                    {
                        vehicleConfigFilePaths.push_back(vehicleFile.path().string());
                        vehicleDropdown->addItem(QString::fromStdString(vehicleConfig.name));
                    }
                    break;
                }
            }
        }
    }

    if (vehicleConfigFilePaths.size() > 0)
    {
        SelectedVehicleChanged(0);
    }

    vehicleDropdown->setEnabled(true);
}

void VehicleSelectionScreen::SelectedVehicleChanged(const int index)
{
    if (index < 0 || index >= vehicleConfigFilePaths.size())
    {
        confirmButton->setEnabled(false);
        return;
    }

    confirmButton->setEnabled(true);
    // TODO: update the thumbnail image as well
    currentSelectedPath = vehicleConfigFilePaths[index];
}
