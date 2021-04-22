#pragma once

#include <memory>

class Image;

struct Material
{
    uint32_t id;
    std::shared_ptr<Image> diffuseImage;
    std::shared_ptr<Image> normalImage;
    std::shared_ptr<Image> specularImage;
};
