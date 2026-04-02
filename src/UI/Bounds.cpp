#include "Bounds.hpp"
#include "../util/util.hpp"

namespace ui {
    
Bounds::Bounds(sf::Vector2f pos, float width, float height) {
    m_pos = pos;
    m_width = width;
    m_height = height;
}

bool Bounds::doesPointIntersect(sf::Vector2f p) {
    const sf::Transform inverse = m_transform.getInverse();
    const sf::Vector2f lp = inverse.transformPoint(p);

    const float left   = m_pos.x;
    const float top    = m_pos.y;
    const float right  = m_pos.x + m_width;
    const float bottom = m_pos.y + m_height;

    if (lp.x < left || lp.x >= right || lp.y < top || lp.y >= bottom)
        return false;
    if (m_radius <= 0.f) {
        return true;
    }

    float r2 = m_radius * m_radius;

    if (lp.x < left + m_radius && lp.y < top + m_radius) {
        return util::getSquaredLengthBetweenPoints(lp, {left + m_radius, top + m_radius}) <= r2;
    }
    if (lp.x > right - m_radius && lp.y < top + m_radius) {
        return util::getSquaredLengthBetweenPoints(lp, {right - m_radius, top + m_radius}) <= r2;
    }
    if (lp.x < left + m_radius && lp.y > bottom - m_radius) {
        return util::getSquaredLengthBetweenPoints(lp, {left + m_radius, bottom - m_radius}) <= r2;
    }
    if (lp.x > right - m_radius && lp.y > bottom - m_radius) {
        return util::getSquaredLengthBetweenPoints(lp, {right - m_radius, bottom - m_radius}) <= r2;
    }
    return true;
}

}