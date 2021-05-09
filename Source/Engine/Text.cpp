#include <filesystem>
#include <stdio.h>
#include <SFML/Graphics/Font.hpp>

#include "Common/FileSystem.h"
#include "Common/Identifier.h"
#include "Image.h"
#include "Text.h"

FontManager::FontManager()
{
    LoadFonts();
}

FontManager::~FontManager()
{
}

bool FontManager::FontExists(const std::string &fontName)
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

    sf::Texture fontTexture = sfFont.getTexture(MAX_FONT_SIZE);
    // Somehow the image size is 128x128, while it should have been 512x512
    int textureWidth = 4 * fontTexture.getSize().x,
        textureHeight = 4 * fontTexture.getSize().y;

    std::string name = std::filesystem::path(fontFilePath).filename().stem().string();
    for (int i = 0; i < NEHE_CHAR_SET.length(); i++)
    {
        wchar_t ch = NEHE_CHAR_SET[i];
        sf::Glyph sfGlyph = sfFont.getGlyph(ch, MAX_FONT_SIZE, false);
        sf::FloatRect boundRect = sfGlyph.bounds;
        sf::IntRect texRect = sfGlyph.textureRect;

        Glyph glyph{};
        glyph.id = i;
        glyph.sizeX = boundRect.width;
        glyph.sizeY = boundRect.height;
        glyph.offsetX = boundRect.left;
        glyph.offsetY = boundRect.top;
        glyph.textureCoordU = static_cast<float>(texRect.left) / textureWidth;
        glyph.textureCoordV = static_cast<float>(texRect.top) / textureHeight;
        glyph.textureCoordUMax = static_cast<float>(texRect.left + texRect.width) / textureWidth;
        glyph.textureCoordVMax = static_cast<float>(texRect.top + texRect.height) / textureHeight;
        glyph.advanceX = sfGlyph.advance;

        font->characters[ch] = glyph;
    }

    fontTexture = sfFont.getTexture(MAX_FONT_SIZE);
    sf::Image fontImage = fontTexture.copyToImage();
    fontImage.saveToFile(FONT_IMAGE_TEMP_FILE);

    font->name = name;
    font->image = std::make_shared<Image>(FONT_IMAGE_TEMP_FILE);
    fontCache[name] = std::move(font);

    return true;
}

void FontManager::LoadFonts()
{
    for (std::string fontFilePath : FileSystem::GetFontFiles())
    {
        LoadFont(fontFilePath);
    }
}

bool FontManager::GenerateTextMesh(const Text &text, ScreenMesh &screenMesh)
{
    if (!FontExists(text.fontName))
    {
        return false;
    }

    const std::unique_ptr<Font> &font = fontCache[text.fontName];
    std::vector<ScreenObjectVertex> &vertices = screenMesh.vertices;

    // This assumes that the origin (0, 0) of the screen is at the left top corner
    glm::vec3 color = text.color;
    float scale = text.fontSize * FONT_SIZE_SCREEN_SCALE;
    float positionX = text.position.x,
          positionY = text.position.y;
    for (const auto &lineText : text.lines)
    {
        std::string::const_iterator chIterator;
        for (chIterator = lineText.begin(); chIterator != lineText.end(); chIterator++)
        {
            wchar_t ch = *chIterator;
            if (font->characters.count(ch) == 0)
            {
                ch = DEFAULT_CHAR;
            }
            const Glyph &glyph = font->characters[ch];

            float chPosX = positionX + glyph.offsetX * scale,
                  chPosY = positionY + (glyph.sizeY + glyph.offsetY) * scale;
            float chWidth = glyph.sizeX * scale,
                  chHeight = glyph.sizeY * scale;

            vertices.push_back({ color, { chPosX, chPosY - chHeight }, { glyph.textureCoordU, glyph.textureCoordV } });
            vertices.push_back({ color, { chPosX, chPosY }, { glyph.textureCoordU, glyph.textureCoordVMax } });
            vertices.push_back({ color, { chPosX + chWidth, chPosY }, { glyph.textureCoordUMax, glyph.textureCoordVMax } });
            vertices.push_back({ color, { chPosX, chPosY - chHeight }, { glyph.textureCoordU, glyph.textureCoordV } });
            vertices.push_back({ color, { chPosX + chWidth, chPosY }, { glyph.textureCoordUMax, glyph.textureCoordVMax } });
            vertices.push_back({ color, { chPosX + chWidth, chPosY - chHeight }, { glyph.textureCoordUMax, glyph.textureCoordV } });

            positionX += glyph.advanceX * scale;
        }

        positionX = text.position.x;
        positionY += LINE_HEIGHT * scale;
    }

    screenMesh.id = Identifier::GenerateIdentifier(IdentifierType::ScreenObject, text.id);
    screenMesh.image = font->image;

    return true;
}
