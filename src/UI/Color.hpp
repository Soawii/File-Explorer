#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>

namespace ui {
    class Color {
    public:
        float h = 0.0f; // 0-360
        float s = 0.0f; // 0-1
        float l = 0.0f; // 0-1
        float a = 1.0f; // 0-1

        Color();
        explicit Color(int r, int g, int b, int a = 255);
        explicit Color(float h, float s, float l, float a = 1.0f);

        Color operator+(const Color& other) const;
        Color operator-(const Color& other) const;
        Color operator*(float scalar) const;
        bool operator!=(const Color& other) const;
        bool operator==(const Color& other) const;
    };

    Color rgbToHsl(const sf::Color& c);
    sf::Color hslToRgb(const Color& hsl);

    Color lerpColor(const Color& c1, const Color& c2, float t);
    sf::Color lerpColor(const sf::Color& c1, const sf::Color& c2, float t);
}