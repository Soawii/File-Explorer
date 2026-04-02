#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <cmath>
#include <string>

namespace util {
    float getSquaredLengthBetweenPoints(sf::Vector2f pos1, sf::Vector2f pos2);
    float getLen(sf::Vector2f vec);
    float timeFunction(const std::function<void(void)>& func);
    sf::Transform interpolateTransform(const sf::Transform& prev, const sf::Transform& curr, float alpha);
    sf::Vector2f normalize(sf::Vector2f v);
    void appendRoundedShadow(sf::VertexArray& verts,
                         sf::FloatRect rect,
                         float radius,
                         float blurRadius,
                         sf::Color color,
                         unsigned int cornerSteps = 12);
    std::string formatBytes(size_t bytes);
    std::string formatDate(const std::string& date);
}