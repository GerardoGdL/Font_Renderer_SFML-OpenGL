#include "Glyph.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <utility>

namespace {
constexpr int CURVE_STEPS = 24;

sf::Vector2f evaluateBezier(
    const sf::Vector2f& p0,
    const sf::Vector2f& p1,
    const sf::Vector2f& p2,
    const sf::Vector2f& p3,
    float t)
{
    const float u = 1.0f - t;

    const float b0 = u * u * u;
    const float b1 = 3.0f * u * u * t;
    const float b2 = 3.0f * u * t * t;
    const float b3 = t * t * t;

    return {
        b0 * p0.x + b1 * p1.x + b2 * p2.x + b3 * p3.x,
        b0 * p0.y + b1 * p1.y + b2 * p2.y + b3 * p3.y
    };
}
}

GlyphLoader::GlyphLoader(std::string directory)
    : fontDirectory(std::move(directory))
{
    if (!fontDirectory.empty() &&
        fontDirectory.back() != '/' &&
        fontDirectory.back() != '\\') {
        fontDirectory += '/';
    }
}

Glyph GlyphLoader::load(char character) const
{
    Glyph glyph;

    const auto code =
        static_cast<unsigned int>(
            static_cast<unsigned char>(character));

    const std::string filename =
        fontDirectory + "gly_" +
        std::to_string(static_cast<unsigned int>(
        static_cast<unsigned char>(character)));

    std::ifstream file(filename);

    if (!file) {
        std::cerr << "Cannot open glyph file: "
                  << filename << '\n';
        return glyph;
    }

    sf::Vector2f current{};
    sf::Vector2f pathStart{};
    bool hasCurrentPoint = false;

    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();

    auto includePoint = [&](const sf::Vector2f& p) {
        minX = std::min(minX, p.x);
        minY = std::min(minY, p.y);
        maxX = std::max(maxX, p.x);
        maxY = std::max(maxY, p.y);
    };

    auto addLine = [&](const sf::Vector2f& a,
                       const sf::Vector2f& b) {
        glyph.segments.push_back({a, b});
        includePoint(a);
        includePoint(b);
    };

    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line)) {
        ++lineNumber;

        std::istringstream input(line);
        char command;

        // Ignore blank lines.
        if (!(input >> command))
            continue;

        if (command == 'M') {
            sf::Vector2f p;

            if (!(input >> p.x >> p.y)) {
                std::cerr << "Invalid M command in "
                          << filename << ':' << lineNumber << '\n';
                return Glyph{};
            }

            current = p;
            pathStart = p;
            hasCurrentPoint = true;
            includePoint(p);
        }
        else if (command == 'L') {
            sf::Vector2f end;

            if (!hasCurrentPoint ||
                !(input >> end.x >> end.y)) {
                std::cerr << "Invalid L command in "
                          << filename << ':' << lineNumber << '\n';
                return Glyph{};
            }

            addLine(current, end);
            current = end;
        }
        else if (command == 'C') {
            sf::Vector2f p1, p2, p3;

            if (!hasCurrentPoint ||
                !(input >> p1.x >> p1.y
                        >> p2.x >> p2.y
                        >> p3.x >> p3.y)) {
                std::cerr << "Invalid C command in "
                          << filename << ':' << lineNumber << '\n';
                return Glyph{};
            }

            // The current point is P0. The file provides P1, P2, P3.
            const sf::Vector2f p0 = current;
            sf::Vector2f previous = p0;

            for (int i = 1; i <= CURVE_STEPS; ++i) {
                const float t =
                    static_cast<float>(i) / CURVE_STEPS;

                const sf::Vector2f next =
                    evaluateBezier(p0, p1, p2, p3, t);

                addLine(previous, next);
                previous = next;
            }

            current = p3;
        }
        else if (command == 'Z') {
            if (!hasCurrentPoint) {
                std::cerr << "Invalid Z command in "
                          << filename << ':' << lineNumber << '\n';
                return Glyph{};
            }

            addLine(current, pathStart);
            current = pathStart;
        }
        else {
            std::cerr << "Unknown command '" << command
                      << "' in " << filename << ':'
                      << lineNumber << '\n';
            return Glyph{};
        }
    }

    if (glyph.segments.empty() ||
        minX == std::numeric_limits<float>::max()) {
        std::cerr << "Glyph has no drawable geometry: "
                  << filename << '\n';
        return Glyph{};
    }

    glyph.minX = minX;
    glyph.maxX = maxX;
    glyph.minY = minY;
    glyph.maxY = maxY;
    glyph.valid = true;

    return glyph;
}