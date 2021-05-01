#include "FileSystem.h"

std::filesystem::path FileSystem::baseDirectory = std::filesystem::current_path();

std::string FileSystem::MainDirectory()
{
    return baseDirectory.string();
}

std::string FileSystem::MapDirectory()
{
    return (baseDirectory / MAP_DIRECTORY_NAME).string();
}

std::string FileSystem::GetMapBaseDirectory(const std::string &mapFilePath)
{
    std::filesystem::path mapFile = mapFilePath;
    return mapFile.parent_path().string();
}

std::string FileSystem::GetHeightMapFile(const std::string &mapBaseDirectory, const std::string &heightMapFileName)
{
    return (std::filesystem::path(mapBaseDirectory) / HEIGHT_MAP_DIRECTORY_NAME / heightMapFileName).string();
}

std::string FileSystem::GetMapBlockFile(const std::string &mapBaseDirectory, const std::string &mapBlockFileName)
{
    return (std::filesystem::path(mapBaseDirectory) / mapBlockFileName).string();
}

std::string FileSystem::GetTextureFile(const std::string &baseDirectory, const std::string &textureFileName)
{
    return (std::filesystem::path(baseDirectory) / TEXTURE_DIRECTORY_NAME / textureFileName).string();
}
