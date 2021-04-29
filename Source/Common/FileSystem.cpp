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
