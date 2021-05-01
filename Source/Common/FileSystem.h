#pragma once

#include <filesystem>
#include <string>

class FileSystem
{
public:
    static constexpr char *HEIGHT_MAP_DIRECTORY_NAME = "heightmaps";
    static constexpr char *MAP_DIRECTORY_NAME = "maps";
    static constexpr char *MAP_FILE_NAME = "map.json";
    static constexpr char *TEXTURE_DIRECTORY_NAME = "textures";

    static std::string MainDirectory();

    static std::string MapDirectory();
    static std::string GetMapBaseDirectory(const std::string &mapFilePath);
    static std::string GetHeightMapFile(const std::string &mapBaseDirectory, const std::string &heightMapFileName);
    static std::string GetMapBlockFile(const std::string &mapBaseDirectory, const std::string &mapBlockFileName);
    static std::string GetTextureFile(const std::string &baseDirectory, const std::string &textureFileName);

private:
    static std::filesystem::path baseDirectory;
};
