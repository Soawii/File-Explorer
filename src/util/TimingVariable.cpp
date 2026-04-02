#include "TimingVariable.hpp"
#include <SFML/Graphics.hpp>
#include "../conf.hpp"
#include <cmath>

sf::Color operator*(const sf::Color& left, float right) {
    return sf::Color(
        sf::Uint8(left.r * right), sf::Uint8(left.g * right),
        sf::Uint8(left.b * right), sf::Uint8(left.a * right));
}

namespace TimingFunctions {
    float linear(float t) {
        return t;
    }
    float easeInQuad(float t) {
        return t * t;
    }
    float easeOutQuad(float t) {
        return t * (2.0f - t);
    }
    float easeInOutQuad(float t) {
        return (t < 0.5f) ? (2.0f * t * t) : (-1.0f + (4.0f - 2.0f * t) * t);
    }
    float smoothstep(float t) {
        return t * t * (3.0f - 2.0f * t);
    }
    float easeInOutSine(float t) {
        return -0.5f * (cosf(conf::constants::PI * t) - 1.0f);
    }
}