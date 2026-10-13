#pragma once

#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

struct LineSegment {
    sf::Vector2f start;
    sf::Vector2f end;
};

struct Glyph {
    std::vector<LineSegment> segments;

    float minX = 0.0f;
    float maxX = 0.0f;
    float minY = 0.0f;
    float maxY = 0.0f;

    bool valid = false;
};

class GlyphLoader {
public:
    explicit GlyphLoader(std::string directory);

    Glyph load(char character) const;

private:
    std::string fontDirectory;
};