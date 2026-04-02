#pragma once
#include <SFML/Graphics.hpp>
#include "../conf.hpp"

namespace ui {

namespace shapes {
    class Shape {
    public:
        virtual void draw(sf::RenderTarget& window, sf::RenderStates states) const = 0;
    };

    class Line : public Shape {
    public:
        Line(sf::Vector2f start, sf::Vector2f end, sf::Color color = sf::Color(255,255,255));
        void draw(sf::RenderTarget& window, sf::RenderStates states) const override;
        sf::VertexArray m_arr;
    };

    class Triangle : public Shape {
    public:
        Triangle(sf::Vector2f tip, float length, float width, float pointingAngle, sf::Color color = sf::Color(255,255,255));
        void draw(sf::RenderTarget& window, sf::RenderStates states) const override;
        sf::VertexArray m_arr;
    };

    class Circle : public Shape {
    public:
        Circle(sf::Vector2f center, float radius, int points = 50, sf::Color color = sf::Color(255,255,255));
        void draw(sf::RenderTarget& window, sf::RenderStates states) const override;
        sf::VertexArray m_arr;
    };

    class HollowCircle : public Shape {
    public:
        HollowCircle(sf::Vector2f center, float outerRadius, float strokeWidth, int points = 50, sf::Color color = sf::Color(255,255,255));
        void draw(sf::RenderTarget& window, sf::RenderStates states) const override;
        sf::VertexArray m_arr;
    };

    class RoundedRect : public Shape {
    public:
        RoundedRect(sf::Vector2f topLeft, float width, float height, float radius = 0.0f, sf::Color color = sf::Color(255,255,255), int points_quarterCircle = 5, bool turnIntoRect = true);
        void draw(sf::RenderTarget& window, sf::RenderStates states) const override;
        sf::VertexArray m_arr;
    };

    class RoundedOutlinedRect : public Shape {
    public:
        RoundedOutlinedRect(sf::Vector2f topLeft, float width, float height, float radius = 0.0f, float outlineWidth = 0.0f, sf::Color color = sf::Color(255,255,255), sf::Color outlineColor = sf::Color(255,255,255), sf::Color outlineColorEnd = sf::Color(255,255,255), int points_quarterCircle = 5);
        void draw(sf::RenderTarget& window, sf::RenderStates states) const override;
        sf::VertexArray m_inner, m_outer;
    };

    class Arrow : public Shape {
    public:
        Arrow(sf::Vector2f tip, float triangleLength, float triangleWidth, float reactangleLength, float reactangleWidth, float angleRadians = 0.0f, sf::Color color = sf::Color::White);

        void draw(sf::RenderTarget& window, sf::RenderStates states) const override;
        
        sf::VertexArray m_triangle;
        sf::VertexArray m_base;
    };
}

}