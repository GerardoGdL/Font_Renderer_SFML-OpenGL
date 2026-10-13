#include "TextRenderer.h"

#include <algorithm>
#include <iostream>
#include <limits>

namespace {
constexpr float CHARACTER_GAP = 0.06f;
constexpr float SPACE_ADVANCE = 0.35f;
}

TextRenderer::TextRenderer(const std::string& fontDirectory)
    : loader(fontDirectory)
{
}

void TextRenderer::includeGlyph(unsigned char character)
{
    if (cache.find(character) == cache.end()) {
        cache.emplace(character,
                      loader.load(static_cast<char>(character)));
    }
}

bool TextRenderer::setText(const std::string& text)
{
    segments.clear();

    minX = 0.0f;
    maxX = 0.0f;
    minY = std::numeric_limits<float>::max();
    maxY = std::numeric_limits<float>::lowest();

    float cursorX = 0.0f;
    bool foundGlyph = false;

    for (unsigned char character : text) {
        if (character == ' ') {
            cursorX += SPACE_ADVANCE;
            continue;
        }

        // Load each unique character
        includeGlyph(character);

        const auto it = cache.find(character);

        if (it == cache.end() || !it->second.valid) {
            std::cerr << "Skipping unsupported character code "
                      << static_cast<int>(character) << '\n';

            cursorX += SPACE_ADVANCE;
            continue;
        }

        const Glyph& glyph = it->second;
        const float glyphWidth = glyph.maxX - glyph.minX;

        // Align the glyph's left geometric bound to the cursor.
        const float offsetX = cursorX - glyph.minX;

        for (const LineSegment& segment : glyph.segments) {
            segments.push_back({
                {segment.start.x + offsetX, segment.start.y},
                {segment.end.x + offsetX, segment.end.y}
            });
        }

        minY = std::min(minY, glyph.minY);
        maxY = std::max(maxY, glyph.maxY);

        cursorX += glyphWidth + CHARACTER_GAP;
        foundGlyph = true;
    }

    maxX = cursorX;

    if (!foundGlyph || segments.empty()) {
        std::cerr << "The input string contains no drawable glyphs.\n";
        return false;
    }

    // Remove the unnecessary gap after the final character.
    maxX -= CHARACTER_GAP;

    if (maxX <= minX)
        maxX = minX + 1.0f;

    if (maxY <= minY)
        maxY = minY + 1.0f;

    return true;
}

float TextRenderer::width() const
{
    return maxX - minX;
}

float TextRenderer::height() const
{
    return maxY - minY;
}

float TextRenderer::fitScale(sf::Vector2u viewport, float margin) const
{
    const float availableWidth =
        std::max(1.0f,
            static_cast<float>(viewport.x) - 2.0f * margin);

    const float availableHeight =
        std::max(1.0f,
            static_cast<float>(viewport.y) - 2.0f * margin);

    const float scaleX = availableWidth / width();
    const float scaleY = availableHeight / height();

    // Uniform scaling preserves the original proportions.
    return std::min(scaleX, scaleY);
}

void TextRenderer::draw(sf::RenderTarget& target, float scale, float scrollX, float margin) const
{
    (void)margin;

    if (segments.empty())
        return;

    const sf::Vector2u viewport = target.getSize();

    // Center the string in the viewport before scrolling.
    const float originX =
        (static_cast<float>(viewport.x) - width() * scale)
        / 2.0f + scrollX;

    const float originY =
        (static_cast<float>(viewport.y) - height() * scale)
        / 2.0f;

    sf::VertexArray lines(sf::PrimitiveType::Lines);

    for (const LineSegment& segment : segments) {
        // Glyph coordinates have Y increasing upwards.
        // Screen coordinates have Y increasing downwards.
        const sf::Vector2f a(
            originX + (segment.start.x - minX) * scale,
            originY + (maxY - segment.start.y) * scale);

        const sf::Vector2f b(
            originX + (segment.end.x - minX) * scale,
            originY + (maxY - segment.end.y) * scale);

        sf::Vertex vertexA;
        vertexA.position = a;
        vertexA.color = sf::Color::White;

        sf::Vertex vertexB;
        vertexB.position = b;
        vertexB.color = sf::Color::White;

        lines.append(vertexA);
        lines.append(vertexB);
    }

    target.draw(lines);
}