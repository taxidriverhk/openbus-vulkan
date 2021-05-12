#include <algorithm>
#include <stdio.h>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

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

std::vector<uint8_t> Image::GetDominantColor() const
{
    std::vector<uint8_t> dominantColor(4, 0U);
    if (pixels)
    {
        int sampleWidth = 1,
            sampleHeight = 1;
        std::vector<uint8_t> samplePixels(4);
        stbir_resize_uint8(pixels, width, height, 0, samplePixels.data(), sampleWidth, sampleHeight, 0, channels);

        dominantColor[0] = samplePixels[0];
        dominantColor[1] = samplePixels[1];
        dominantColor[2] = samplePixels[2];
    }

    return dominantColor;
}

bool Image::Load(const std::string &path, ImageColor color)
{
    Destroy();

    this->pixels = stbi_load(
        path.c_str(),
        &width,
        &height,
        nullptr,
        color == ImageColor::ColorWithAlpha ? STBI_rgb_alpha : STBI_grey);
    this->channels = color == ImageColor::ColorWithAlpha ? 4 : 1;
    this->path = path;
    return pixels != nullptr;
}
