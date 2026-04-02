#pragma once
#include <SFML/Graphics.hpp>

namespace ui {
    class Bounds {
    public:
        Bounds() = default;
        Bounds(sf::Vector2f pos, float width, float height);

        bool doesPointIntersect(sf::Vector2f p);

        sf::Vector2f m_pos = {0.0f, 0.0f};
        float m_width = 0.0f;
        float m_height = 0.0f;
        float m_radius = 0.0f;

        sf::Transform m_transform;
    };
}