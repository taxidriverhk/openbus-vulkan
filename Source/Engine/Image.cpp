#include <stdio.h>
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

Image::Image(const std::string &path, ImageColor color)
    : Image()
{
    if (!Load(path, color))
    {
        throw std::runtime_error("Failed to load image from " + path);
    }
}

Image::Image(const uint8_t *srcPixels, int width, int height)
    : width(width),
      height(height),
      channels(4)
{
    uint32_t pixelSize = channels * width * height;
    pixels = (uint8_t *) malloc(pixelSize);
    if (pixels)
    {
        memcpy(pixels, srcPixels, pixelSize);
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

bool Image::Load(const std::string &path, ImageColor color)
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
