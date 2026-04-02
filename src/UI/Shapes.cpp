#include "Shapes.hpp"
#include "../conf.hpp"
#include "../util/util.hpp"

namespace ui {

namespace shapes {
    Line::Line(sf::Vector2f start, sf::Vector2f end, sf::Color color) : m_arr(sf::Lines, 2) {
        m_arr.setPrimitiveType(sf::Lines);
        m_arr.resize(2);
        m_arr[0].position = start;
        m_arr[1].position = end;
        m_arr[0].color = color;
        m_arr[1].color = color;
    }
    void Line::draw(sf::RenderTarget& window, sf::RenderStates states) const {
        window.draw(m_arr, states);
    }

    Triangle::Triangle(
        sf::Vector2f tip, 
        float length, 
        float width, 
        float angleRadians, 
        sf::Color color)
    : m_arr(sf::TriangleFan, 0)
    {
        angleRadians = angleRadians * 180.0f / conf::constants::PI;
        bool pointingRight = (angleRadians < 90.0f || angleRadians > 270.0f);
    
        // Three corners of the triangle
        sf::Vector2f p1 = tip; // The tip
        sf::Vector2f p2, p3;

        if (pointingRight) {
            // Tip points right, base is vertical on the left
            p2 = tip + sf::Vector2f(-length, -width / 2.0f); // Top of base
            p3 = tip + sf::Vector2f(-length, width / 2.0f);  // Bottom of base
        } else {
            // Tip points left, base is vertical on the right
            p2 = tip + sf::Vector2f(length, -width / 2.0f); // Top of base
            p3 = tip + sf::Vector2f(length, width / 2.0f);  // Bottom of base
        }
    
        m_arr.resize(3);
        m_arr[0] = sf::Vertex(p1, color);
        m_arr[1] = sf::Vertex(p2, color);
        m_arr[2] = sf::Vertex(p3, color);
    }

    void Triangle::draw(sf::RenderTarget& window, sf::RenderStates states) const {
        window.draw(m_arr, states);
    }

    Circle::Circle(
        sf::Vector2f center, 
        float radius, 
        int points, 
        sf::Color color)
    : m_arr(sf::TriangleFan, points) 
    {
        float angle = 0;
        float d_angle = 2 * conf::constants::PI / points;
        for (int i = 0; i < points; i++) {
            m_arr[i].position = {center.x + radius * cosf(angle), center.y + radius * sinf(angle)};
            angle += d_angle;
        }
        for (int i = 0; i < points; i++) {
            m_arr[i].color = color;
        }
    }
    void Circle::draw(sf::RenderTarget& window, sf::RenderStates states) const {
        window.draw(m_arr, states);
    }

    HollowCircle::HollowCircle(
        sf::Vector2f center, 
        float outerRadius, 
        float strokeWidth, 
        int points, 
        sf::Color color) 
    : m_arr(sf::TriangleStrip, points * 2 + 2) 
    {
        float innerRadius = outerRadius - strokeWidth;

        float angle = 0;
        float d_angle = 2 * conf::constants::PI / points;
        for (int i = 0; i < points; i++) {
            m_arr[2 * i].position = {center.x + innerRadius * cosf(angle), center.y + innerRadius * sinf(angle)};
            m_arr[2 * i + 1].position = {center.x + outerRadius * cosf(angle), center.y + outerRadius * sinf(angle)};
            m_arr[2 * i].color = color;
            m_arr[2 * i + 1].color = color;
            angle += d_angle;
        }
        m_arr[points * 2] = m_arr[0];
        m_arr[points * 2 + 1] = m_arr[1];
    }
    void HollowCircle::draw(sf::RenderTarget& window, sf::RenderStates states) const {
        window.draw(m_arr, states);
    }

    RoundedRect::RoundedRect(
        sf::Vector2f topLeft, 
        float width, 
        float height, 
        float radius,
        sf::Color color,
        int points_quarterCircle,
        bool turnIntoRect) 
    : m_arr(sf::TriangleFan)
    {
        radius = std::max(0.0f, std::min(radius, std::min(width / 2.0f, height / 2.0f)));
        if (turnIntoRect && fabs(radius) < 0.5f) {
            m_arr.resize(4);
            m_arr[0].position = topLeft;
            m_arr[1].position = {topLeft.x + width, topLeft.y};
            m_arr[2].position = {topLeft.x + width, topLeft.y + height};
            m_arr[3].position = {topLeft.x, topLeft.y + height};
            m_arr[0].color = color;
            m_arr[1].color = color;
            m_arr[2].color = color;
            m_arr[3].color = color;
        }
        else {
            m_arr.resize(points_quarterCircle * 4);
            const sf::Vector2f centerTopLeft = {topLeft.x + radius, topLeft.y + radius};
            const sf::Vector2f centerTopRight = {topLeft.x + width - radius, topLeft.y + radius};
            const sf::Vector2f centerBottomRight = {topLeft.x + width - radius, topLeft.y + height - radius};
            const sf::Vector2f centerBottomLeft = {topLeft.x + radius, topLeft.y + height - radius};

            int idx = 0;
            float angle = conf::constants::PI;
            const float d_angle = -(conf::constants::PI / 2) / (points_quarterCircle - 1);

            for (int i = 0; i < points_quarterCircle; i++) {
                m_arr[idx++].position = {centerTopLeft.x + cosf(angle) * radius, centerTopLeft.y - sinf(angle) * radius};
                angle += d_angle;
            }
            angle = conf::constants::PI / 2;
            for (int i = 0; i < points_quarterCircle; i++) {
                m_arr[idx++].position = {centerTopRight.x + cosf(angle) * radius, centerTopRight.y - sinf(angle) * radius};
                angle += d_angle;
            }
            angle = 0.0f;
            for (int i = 0; i < points_quarterCircle; i++) {
                m_arr[idx++].position = {centerBottomRight.x + cosf(angle) * radius, centerBottomRight.y - sinf(angle) * radius};
                angle += d_angle;
            }
            angle = conf::constants::PI * 3 / 2;
            for (int i = 0; i < points_quarterCircle; i++) {
                m_arr[idx++].position = {centerBottomLeft.x + cosf(angle) * radius, centerBottomLeft.y - sinf(angle) * radius};
                angle += d_angle;
            }
            for (int i = 0; i < idx; i++) {
                m_arr[i].color = color;
            }
        }
    }

    void RoundedRect::draw(sf::RenderTarget& window, sf::RenderStates states) const {
        window.draw(m_arr, states);
    }

    RoundedOutlinedRect::RoundedOutlinedRect(
        sf::Vector2f topLeft, 
        float width, 
        float height, 
        float radius,
        float outlineWidth, 
        sf::Color color,
        sf::Color outlineColor,
        sf::Color outlineColorEnd,
        int points_quarterCircle) 
    {
        if (fabs(outlineWidth) < 0.5f) {
            m_inner = RoundedRect(
                topLeft, width, height, radius, 
                color, points_quarterCircle).m_arr;
        }
        else {
            if (std::max(0.0f, radius - outlineWidth) < 0.5f && radius < 0.5f) {
                m_inner = RoundedRect(
                    sf::Vector2f(topLeft.x + outlineWidth, topLeft.y + outlineWidth), 
                    std::max(0.0f, width - outlineWidth * 2), 
                    std::max(0.0f, height - outlineWidth * 2), 
                    std::max(0.0f, radius - outlineWidth),
                    color, points_quarterCircle).m_arr;

                const sf::VertexArray& outer_arr = RoundedRect(
                    topLeft, width, height, radius, 
                    outlineColor, points_quarterCircle).m_arr;

                m_outer.setPrimitiveType(sf::TriangleStrip);
                m_outer.resize(m_inner.getVertexCount() * 2 + 2);
                for (int i = 0; i < m_inner.getVertexCount(); i++) {
                    m_outer[2 * i].position = m_inner[i].position;
                    m_outer[2 * i + 1].position = outer_arr[i].position;
                    m_outer[2 * i].color = outlineColor;
                    m_outer[2 * i + 1].color = outlineColorEnd;
                }
                m_outer[m_inner.getVertexCount() * 2] = m_outer[0];
                m_outer[m_inner.getVertexCount() * 2 + 1] = m_outer[1];
            }
            else {
                m_inner = RoundedRect(
                    sf::Vector2f(topLeft.x + outlineWidth, topLeft.y + outlineWidth), 
                    std::max(0.0f, width - outlineWidth * 2), 
                    std::max(0.0f, height - outlineWidth * 2), 
                    std::max(0.0f, radius - outlineWidth),
                    color, points_quarterCircle, false).m_arr;

                const sf::VertexArray& outer_arr = RoundedRect(
                    topLeft, width, height, radius, 
                    outlineColor, points_quarterCircle, false).m_arr;

                m_outer.setPrimitiveType(sf::TriangleStrip);
                m_outer.resize(m_inner.getVertexCount() * 2 + 2);
                for (int i = 0; i < m_inner.getVertexCount(); i++) {
                    m_outer[2 * i].position = m_inner[i].position;
                    m_outer[2 * i + 1].position = outer_arr[i].position;
                    m_outer[2 * i].color = outlineColor;
                    m_outer[2 * i + 1].color = outlineColorEnd;
                }
                m_outer[points_quarterCircle * 4 * 2] = m_outer[0];
                m_outer[points_quarterCircle * 4 * 2 + 1] = m_outer[1];
            }
        }
    }

    void RoundedOutlinedRect::draw(sf::RenderTarget& window, sf::RenderStates states) const {
        if (m_outer.getVertexCount() > 0)
            window.draw(m_outer, states);
        window.draw(m_inner, states);
    }

    Arrow::Arrow(sf::Vector2f tip, float triangleLength, float triangleWidth, float reactangleLength, float reactangleWidth, float angleRadians, sf::Color color)
    {
        bool pointingRight = (fabs(angleRadians) < 0.01f);
    
        Triangle head(tip, triangleLength, triangleWidth, angleRadians, color);
        m_triangle = head.m_arr;
        
        sf::Vector2f shaftTopLeft;
        
        if (pointingRight) {
            shaftTopLeft = sf::Vector2f(
                tip.x - triangleLength - reactangleLength,
                tip.y - reactangleWidth / 2.0f
            );
        } else {
            shaftTopLeft = sf::Vector2f(
                tip.x + triangleLength,
                tip.y - reactangleWidth / 2.0f
            );
        }
        
        RoundedRect shaft(
            shaftTopLeft,
            reactangleLength,
            reactangleWidth,
            0.0f,
            color
        );
        m_base = shaft.m_arr;
    }

    void Arrow::draw(sf::RenderTarget& window, sf::RenderStates states) const {
        window.draw(m_base, states);
        window.draw(m_triangle, states);
    }

}

}