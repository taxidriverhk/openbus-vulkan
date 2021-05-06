#pragma once

#include <string>

enum class ImageColor
{
    ColorWithAlpha,
    Grayscale
};

class Image
{
public:
    Image();
    Image(const std::string &path, ImageColor color = ImageColor::ColorWithAlpha);
    Image(const uint8_t *srcPixels, int width, int height);
    ~Image();

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    uint8_t *GetPixels() const { return pixels; }
    bool Load(const std::string &path, ImageColor color);

private:
    void Destroy();

    std::string path;
    int width;
    int height;
    int channels;
    uint8_t *pixels;
};
