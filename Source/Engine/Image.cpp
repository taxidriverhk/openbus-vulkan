#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Image.h"

Image::Image(std::string path)
    : path(path),
      pixels(nullptr),
      width(0),
      height(0),
      channels(0)
{
    Load(path);
}

Image::~Image()
{
    Destroy();
}

void Image::Destroy()
{
    if (pixels)
    {
        stbi_image_free(pixels);
    }
}

bool Image::Load(std::string path)
{
    Destroy();

    this->pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    this->path = path;
    return pixels != nullptr;
}
