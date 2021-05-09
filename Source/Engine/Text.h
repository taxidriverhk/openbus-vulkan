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
    glm::vec2 position;
    glm::vec3 color;
    std::string fontName;
    std::vector<std::string> lines;
};

class FontManager
{
public:
    FontManager();
    ~FontManager();

    bool FontExists(const std::string &fontName);
    Font *GetFont(const std::string &fontName);
    bool GenerateTextMesh(const Text &text, ScreenMesh &screenMesh);

private:
    static constexpr std::wstring_view NEHE_CHAR_SET
        = L" \t\r\nABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890\"!`?'.,;:()[]{}<>|/@\\^$-%+=#_&~*";
    static constexpr char *FONT_IMAGE_TEMP_FILE = "temp/fontImage.bmp";
    static constexpr wchar_t DEFAULT_CHAR = '_';
    static constexpr float FONT_SIZE_SCREEN_SCALE = 0.05f;
    static constexpr int MAX_FONT_SIZE = 72;
    static constexpr int LINE_HEIGHT = 70;

    void LoadFonts();
    bool LoadFont(const std::string &fontFilePath);

    std::unordered_map<std::string, std::unique_ptr<Font>> fontCache;
};
