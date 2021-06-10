#pragma once

#include <filesystem>
#include <string>

class FileSystem
{
public:
    static constexpr char *MAP_FILE_NAME = "map.json";
    static constexpr char *SETTINGS_FILE_NAME = "settings.json";
    static constexpr char *VEHICLE_FILE_NAME = "vehicle.json";

    static constexpr char *FONT_DIRECTORY_NAME = "fonts";
    static constexpr char *HEIGHT_MAP_DIRECTORY_NAME = "heightmaps";
    static constexpr char *MAP_DIRECTORY_NAME = "maps";
    static constexpr char *MODEL_DIRECTORY_NAME = "models";
    static constexpr char *OBJECT_DIRECTORY_NAME = "objects";
    static constexpr char *ROADS_DIRECTORY_NAME = "roads";
    static constexpr char *TEXTURE_DIRECTORY_NAME = "textures";
    static constexpr char *VEHICLE_DIRECTORY_NAME = "vehicles";

    static std::string MainDirectory();

    static std::string MapDirectory();
    static std::string VehicleDirectory();
    static std::list<std::string> GetFontFiles();
    static std::string GetParentDirectory(const std::string &mapFilePath);
    static std::string GetHeightMapFile(const std::string &mapBaseDirectory, const std::string &heightMapFileName);
    static std::string GetMapBlockFile(const std::string &mapBaseDirectory, const std::string &mapBlockFileName);
    static std::string GetGameObjectFile(const std::string &gameObjectBaseDirectory, const std::string &objectFileName);
    static std::string GetRoadObjectFile(const std::string &objectFileName);
    static std::string GetStaticObjectFile(const std::string &objectFileName);
    static std::string GetModelFile(const std::string &baseDirectory, const std::string &modelFileName);
    static std::string GetSettingsFile();
    static std::string GetTextureFile(const std::string &baseDirectory, const std::string &textureFileName);
    static std::string GetVehicleFile(const std::string &vehicleBaseDirectory);

private:
    static std::filesystem::path baseDirectory;
};
