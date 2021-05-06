#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Mesh.h"

struct ScreenMesh;
class Image;

struct Glyph
{
    uint8_t id;
    float textureCoordU;
    float textureCoordV;
    float textureCoordUMax;
    float textureCoordVMax;
    float offsetX;
    float offsetY;
    float sizeX;
    float sizeY;
    float advanceX;
};

struct Font
{
    std::string name;
    std::shared_ptr<Image> image;
    std::unordered_map<wchar_t, Glyph> characters;
};

struct Text
{
    uint32_t id;
    float fontSize;
    float lineWidth;
    glm::vec2 position;
    glm::vec3 color;
    std::string fontName;
    std::string text;
};

class FontManager
{
public:
    FontManager();
    ~FontManager();

    bool FindFont(const std::string &fontName);
    Font *GetFont(const std::string &fontName);
    bool LoadFont(const std::string &fontFilePath);
    bool GenerateTextMesh(const Text &text, ScreenMesh &screenMesh);

private:
    static constexpr std::wstring_view NEHE_CHAR_SET
        = L" \t\r\nABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890\"!`?'.,;:()[]{}<>|/@\\^$-%+=#_&~*";
    static constexpr int MAX_FONT_SIZE = 72;

    std::unordered_map<std::string, std::unique_ptr<Font>> fontCache;
};
