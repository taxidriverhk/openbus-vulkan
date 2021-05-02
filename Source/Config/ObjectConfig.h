#pragma once

#include <string>

struct StaticObjectConfig
{
    std::string name;
    std::string mesh;

    std::string diffuseMaterial;
    std::string specularMaterial;
    std::string alphaMaterial;
};
