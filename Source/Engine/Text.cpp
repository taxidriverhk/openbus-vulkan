#include <filesystem>
#include <SFML/Graphics/Font.hpp>
#include "Image.h"
#include "Text.h"

FontManager::FontManager()
{
}

FontManager::~FontManager()
{
}

bool FontManager::FindFont(const std::string &fontName)
{
    return fontCache.count(fontName) != 0;
}

Font *FontManager::GetFont(const std::string &fontName)
{
    if (fontCache.count(fontName) == 0)
    {
        return nullptr;
    }
    return fontCache[fontName].get();
}

bool FontManager::LoadFont(const std::string &fontFilePath)
{
    std::unique_ptr<Font> font = std::make_unique<Font>();

    sf::Font sfFont;
    if (!sfFont.loadFromFile(fontFilePath))
    {
        return false;
    }

    std::string name = std::filesystem::path(fontFilePath).filename().string();
    for (int i = 0; i < NEHE_CHAR_SET.length(); i++)
    {
        wchar_t ch = NEHE_CHAR_SET[i];
        sf::Glyph sfGlyph = sfFont.getGlyph(ch, MAX_FONT_SIZE, false);
        Glyph glyph{};
    }
    
    sf::Texture fontTexture = sfFont.getTexture(MAX_FONT_SIZE);
    sf::Image fontImage = fontTexture.copyToImage();

    font->name = name;
    font->image = std::make_shared<Image>(
        fontImage.getPixelsPtr(),
        fontImage.getSize().x,
        fontImage.getSize().y);
    fontCache[name] = std::move(font);

    return true;
}

bool FontManager::GenerateTextMesh(const Text &text, ScreenMesh &screenMesh)
{
    return false;
}
