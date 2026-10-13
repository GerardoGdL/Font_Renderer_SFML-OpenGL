
#include <SFML/Graphics.hpp>

#include <iostream>
#include <string>

#include "TextRenderer.h"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr
            << "No text provided. Please provide a string to render as a command line argument.\n";
        return 1;
    }

    // Support a quoted string, and also join multiple arguments
    // if the user forgets to quote the entire string.
    std::string text = argv[1];

    for (int i = 2; i < argc; ++i) {
        text += ' ';
        text += argv[i];
    }

    // This path can be interchanged to any other font directory containing the glyph files.
    TextRenderer renderer("Fonts/cmunss");

    if (!renderer.setText(text))
        return 1;

    sf::ContextSettings settings;
    settings.majorVersion = 2;
    settings.minorVersion = 1;

    sf::RenderWindow window(
        sf::VideoMode({800u, 800u}),
        "Computer Modern Glyph Renderer",
        sf::Style::Default,
        sf::State::Windowed,
        settings
    );

    window.setKeyRepeatEnabled(false);
    window.setVerticalSyncEnabled(true);

    constexpr float MARGIN = 30.0f;
    constexpr float ZOOM_FACTOR = 1.1f;
    constexpr float SCROLL_STEP = 30.0f;

    float zoom = 1.0f;
    float scrollX = 0.0f;

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // This will scroll/scale the text based on the key pressed, R resets to the original position
            if (const auto* key =
                    event->getIf<sf::Event::KeyPressed>()) {
                switch (key->code) {
                    case sf::Keyboard::Key::Escape:
                        window.close();
                        break;

                    case sf::Keyboard::Key::Up:
                        zoom *= ZOOM_FACTOR;
                        break;

                    case sf::Keyboard::Key::Down:
                        zoom /= ZOOM_FACTOR;
                        break;

                    case sf::Keyboard::Key::Left:
                        scrollX -= SCROLL_STEP;
                        break;

                    case sf::Keyboard::Key::Right:
                        scrollX += SCROLL_STEP;
                        break;

                    case sf::Keyboard::Key::R:
                        zoom = 1.0f;
                        scrollX = 0.0f;
                        break;

                    default:
                        break;
                }
            }
        }

        window.clear(sf::Color(26, 26, 26));

        // Recompute the fit scale on every frame so window resizing
        // still starts from the correct viewport dimensions.
        const float baseScale =
            renderer.fitScale(window.getSize(), MARGIN);

        renderer.draw(
            window,
            baseScale * zoom,
            scrollX,
            MARGIN
        );

        window.display();
    }

    return 0;
}