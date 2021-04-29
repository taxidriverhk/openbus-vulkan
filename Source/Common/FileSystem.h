#pragma once

#include <filesystem>
#include <string>

class FileSystem
{
public:
    static constexpr char *MAP_DIRECTORY_NAME = "maps";
    static constexpr char *MAP_FILE_NAME = "map.json";

    static std::string MainDirectory();
    static std::string MapDirectory();

private:
    static std::filesystem::path baseDirectory;
};
