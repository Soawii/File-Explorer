#include "Color.hpp"
#include <iostream>

namespace ui {

Color::Color() = default;

Color::Color(int r, int g, int b, int a) {
    Color color = rgbToHsl(sf::Color(r, g, b, a));
    this->h = color.h;
    this->s = color.s;
    this->l = color.l;
    this->a = color.a;
}
Color::Color(float new_h, float new_s, float new_l, float new_a) {
    this->h = new_h;
    this->s = new_s;
    this->l = new_l;
    this->a = new_a;
}

Color Color::operator+(const Color& other) const {
    return Color(
        std::fmod(h + other.h + 360.f, 360.f),
        s + other.s,
        l + other.l,
        a + other.a
    );
}

Color Color::operator-(const Color& other) const {
    float hueDiff = std::fmod((h - other.h + 540.f), 360.f) - 180.f;
    return Color(
        hueDiff,
        s - other.s,
        l - other.l,
        a - other.a
    );
}

Color Color::operator*(float scalar) const {
    return Color(h * scalar, s * scalar, l * scalar, a * scalar);
}

bool Color::operator!=(const Color& other) const {
    return h != other.h || s != other.s || l != other.l || a != other.a;
}
bool Color::operator==(const Color& other) const {
    return h == other.h && s == other.s && l == other.l && a == other.a;
}

Color rgbToHsl(const sf::Color& c) {
    float r = int(c.r) / 255.f, g = int(c.g) / 255.f, b = int(c.b) / 255.f;
    float max = std::max(r, std::max(g, b));
    float min = std::min(r, std::min(g, b));
    float h = 0.f, s = 0.f;
    float l = (max + min) / 2.f;

    if (max != min) {
        float d = max - min;
        s = l > 0.5f ? d / (2.f - max - min) : d / (max + min);

        if (max == r)
            h = fmod(((g - b) / d + (g < b ? 6.f : 0.f)), 6.f);
        else if (max == g)
            h = (b - r) / d + 2.f;
        else
            h = (r - g) / d + 4.f;

        h *= 60.f;
    }

    return Color(h, s, l, int(c.a) / 255.0f);
}

sf::Color hslToRgb(const Color& hsl) {
    float c = (1.f - std::fabs(2.f * hsl.l - 1.f)) * hsl.s;
    float x = c * (1.f - std::fabs(fmod(hsl.h / 60.f, 2.f) - 1.f));
    float m = hsl.l - c / 2.f;

    float r, g, b;
    if (hsl.h < 60)      { r = c; g = x; b = 0; }
    else if (hsl.h <120) { r = x; g = c; b = 0; }
    else if (hsl.h <180) { r = 0; g = c; b = x; }
    else if (hsl.h <240) { r = 0; g = x; b = c; }
    else if (hsl.h <300) { r = x; g = 0; b = c; }
    else                 { r = c; g = 0; b = x; }

    return sf::Color(
        static_cast<sf::Uint8>((r + m) * 255),
        static_cast<sf::Uint8>((g + m) * 255),
        static_cast<sf::Uint8>((b + m) * 255),
        static_cast<sf::Uint8>(hsl.a * 255)
    );
}

Color lerpColor(const Color& c1, const Color& c2, float t) {
    Color out(0.0f,0.0f,0.0f);
    out.h = c1.h + (c2.h - c1.h) * t;
    out.s = c1.s + (c2.s - c1.s) * t;
    out.l = c1.l + (c2.l - c1.l) * t;
    out.a = c1.a + (c2.a - c1.a) * t;
    return out;
}
sf::Color lerpColor(const sf::Color& c1, const sf::Color& c2, float t) {
    sf::Color out(0,0,0,0);
    out.r = c1.r + (c2.r - c1.r) * t;
    out.g = c1.g + (c2.g - c1.g) * t;
    out.b = c1.b + (c2.b - c1.b) * t;
    out.a = c1.a + (c2.a - c1.a) * t;
    return out;
}

}