#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Image.h"

Image::Image()
    : pixels(nullptr),
      width(0),
      height(0),
      channels(0)
{
}

Image::Image(std::string path, ImageColor color)
    : Image()
{
    if (!Load(path, color))
    {
        throw std::runtime_error("Failed to load image from " + path);
    }
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

bool Image::Load(std::string path, ImageColor color)
{
    Destroy();

    this->pixels = stbi_load(
        path.c_str(),
        &width,
        &height,
        &channels,
        color == ImageColor::ColorWithAlpha ? STBI_rgb_alpha : STBI_grey);
    this->path = path;
    return pixels != nullptr;
}
