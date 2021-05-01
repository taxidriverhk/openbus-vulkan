#pragma once

#include <string>

struct StaticObjectConfig
{
    std::string name;
    std::string meshFilePath;

    std::string diffuseMaterialFilePath;
    std::string specularMaterialFilePath;
    std::string alphaMaterialFilePath;
};
