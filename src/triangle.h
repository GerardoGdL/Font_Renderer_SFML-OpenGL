#pragma once
#include <SFML/Graphics.hpp>
#include <glad/gl.h>
#include <vector>
#include <iostream>

class Triangle : public sf::Drawable {
private:

    // 2d control points for the bezier curve
    std::vector<sf::Vector2f> vertices;

    // shader that will be used to render the bezier curve
    sf::Shader triShader;

    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

public:

    Triangle(const std::vector<sf::Vector2f>& points) : vertices(points) 
    {
        if (!triShader.loadFromFile(
            "../../src/triangle.vert", 
            "../../src/triangle.frag") ) {
                std::cerr << "Failed to load shader for triangle " << std::endl;
            }
    }
        
    void setVertices(const std::vector<sf::Vector2f>& points) {
        vertices = points;
    }

};