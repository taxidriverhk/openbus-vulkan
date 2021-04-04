#pragma once

#include <string>

class Image
{
public:
    Image(std::string path);
    ~Image();

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    uint8_t *GetPixels() const { return pixels; }

private:
    void Destroy();
    bool Load(std::string path);

    std::string path;
    int width;
    int height;
    int channels;
    uint8_t *pixels;
};
