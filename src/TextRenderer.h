
#pragma once

#include "Glyph.h"

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <vector>

class TextRenderer {
public:
    explicit TextRenderer(const std::string& fontDirectory);

    bool setText(const std::string& text);

    // Calculate the uniform scale that fits the text in a viewport.
    float fitScale(sf::Vector2u viewport, float margin) const;

    // Draw the text using scale and horizontal offset.
    void draw(sf::RenderTarget& target,
              float scale,
              float scrollX,
              float margin) const;

    float width() const;
    float height() const;

private:
    GlyphLoader loader;
    std::unordered_map<unsigned char, Glyph> cache;
    std::vector<LineSegment> segments;

    float minX = 0.0f;
    float maxX = 1.0f;
    float minY = 0.0f;
    float maxY = 1.0f;

    void includeGlyph(unsigned char character);
};