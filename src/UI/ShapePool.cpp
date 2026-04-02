#include "ShapePool.hpp"
#include <iostream>
  
namespace ui {

RoundedRectParams::RoundedRectParams(float width_, float height_, float borderRadius_, size_t quarterPoints_) 
: width(std::max(0.0f, width_)), height(std::max(0.0f, height_)), borderRadius(std::max(0.0f, borderRadius_)), quarterPoints(quarterPoints_) {}
bool RoundedRectParams::operator==(const RoundedRectParams& o) const
{
    return width == o.width && height == o.height && borderRadius == o.borderRadius && quarterPoints == o.quarterPoints;
}  
size_t RoundedRectParamsHasher::operator()(const RoundedRectParams& s) const {
    return (((s.width * size_t(10000)) + s.height) * size_t(1000) + s.borderRadius) * size_t(1000) + s.quarterPoints;
}

RoundedOutlinedRectParams::RoundedOutlinedRectParams(float width_, float height_, float borderWidth_, float borderRadius_, size_t quarterPoints_) 
: width(std::max(0.0f, width_)), height(std::max(0.0f, height_)), borderWidth(std::max(0.0f, borderWidth_)), borderRadius(std::max(0.0f, borderRadius_)), quarterPoints(quarterPoints_) {}
bool RoundedOutlinedRectParams::operator==(const RoundedOutlinedRectParams& o) const
{
    return width == o.width && height == o.height && borderWidth == o.borderWidth && borderRadius == o.borderRadius;
}  
size_t RoundedOutlinedRectParamsHasher::operator()(const RoundedOutlinedRectParams& s) const {
    return (((s.width * size_t(10000) + s.height) * size_t(1000) + s.borderWidth) * size_t(1000) + s.borderRadius) * size_t(1000) + s.quarterPoints;
}

std::unordered_map<RoundedRectParams, shapes::RoundedRect*, RoundedRectParamsHasher> ShapePool::m_roundedRects;
std::unordered_map<RoundedOutlinedRectParams, shapes::RoundedOutlinedRect*, RoundedOutlinedRectParamsHasher> ShapePool::m_roundedOutlinedRects;

void ShapePool::addShape(const RoundedRectParams& p, shapes::RoundedRect* shape) {
    m_roundedRects[p] = shape;
}
void ShapePool::addShape(const RoundedOutlinedRectParams& p, shapes::RoundedOutlinedRect* shape) {
    m_roundedOutlinedRects[p] = shape;
}

shapes::RoundedRect* ShapePool::findRoundedRect(const RoundedRectParams& p, const sf::Color& fillColor) {
    const auto it = m_roundedRects.find(p);
    if (it == m_roundedRects.end())
        return nullptr;
    shapes::RoundedRect* shape = it->second;
    if (shape->m_arr.getVertexCount() > 0 && shape->m_arr[0].color != fillColor) {
        for (size_t i = 0; i < shape->m_arr.getVertexCount(); i++) {
            shape->m_arr[i].color = fillColor;
        }
    }
    return shape;
}
shapes::RoundedRect* ShapePool::findOrCreateRoundedRect(const RoundedRectParams& p, const sf::Color& fillColor) {
    shapes::RoundedRect* shape = findRoundedRect(p, fillColor);
    if (shape == nullptr) {
        shape = new shapes::RoundedRect({0.0f, 0.0f}, p.width, p.height, p.borderRadius, fillColor, 5);
        addShape(p, shape);
    }
    return shape;
}
shapes::RoundedOutlinedRect* ShapePool::findRoundedOutlinedRect(const RoundedOutlinedRectParams& p, const sf::Color& fillColor, const sf::Color& borderColor) {
    const auto it = m_roundedOutlinedRects.find(p);
    if (it == m_roundedOutlinedRects.end())
        return nullptr;
    shapes::RoundedOutlinedRect* shape = it->second;
    if (shape->m_inner.getVertexCount() > 0 && shape->m_inner[0].color != fillColor) {
        for (size_t i = 0; i < shape->m_inner.getVertexCount(); i++) {
            shape->m_inner[i].color = fillColor;
        }
    }
    if (shape->m_outer.getVertexCount() > 0 && shape->m_outer[0].color != borderColor) {
        for (size_t i = 0; i < shape->m_outer.getVertexCount(); i++) {
            shape->m_outer[i].color = borderColor;
        }
    }
    return shape;
}
shapes::RoundedOutlinedRect* ShapePool::findRoundedOutlinedRect(const RoundedOutlinedRectParams& p, const sf::Color& fillColor, const sf::Color& borderColor, const sf::Color& borderColorEnd) {
    const auto it = m_roundedOutlinedRects.find(p);
    if (it == m_roundedOutlinedRects.end())
        return nullptr;
    shapes::RoundedOutlinedRect* shape = it->second;
    if (shape->m_inner.getVertexCount() > 0 && shape->m_inner[0].color != fillColor) {
        for (size_t i = 0; i < shape->m_inner.getVertexCount(); i++) {
            shape->m_inner[i].color = fillColor;
        }
    }
    if (shape->m_outer.getVertexCount() > 0 && shape->m_outer[0].color != borderColor) {
        for (size_t i = 0; i < shape->m_outer.getVertexCount() / 2; i++) {
            shape->m_outer[2 * i].color = borderColor;
        }
    }
    if (shape->m_outer.getVertexCount() > 0 && shape->m_outer[1].color != borderColorEnd) {
        for (size_t i = 0; i < shape->m_outer.getVertexCount() / 2; i++) {
            shape->m_outer[2 * i + 1].color = borderColorEnd;
        }
    }
    return shape;
}
shapes::RoundedOutlinedRect* ShapePool::findOrCreateRoundedOutlinedRect(const RoundedOutlinedRectParams& p, const sf::Color& fillColor, const sf::Color& borderColor) {
    shapes::RoundedOutlinedRect* shape = findRoundedOutlinedRect(p, fillColor, borderColor);
    if (shape == nullptr) {
        shape = new shapes::RoundedOutlinedRect({0.0f, 0.0f}, p.width, p.height, p.borderRadius, p.borderWidth, fillColor, borderColor, borderColor, 6);
        addShape(p, shape);
    }
    return shape;
}

shapes::RoundedOutlinedRect* ShapePool::findOrCreateRoundedOutlinedRect(const RoundedOutlinedRectParams& p, const sf::Color& fillColor, const sf::Color& borderColor, const sf::Color& borderColorEnd) {
    shapes::RoundedOutlinedRect* shape = findRoundedOutlinedRect(p, fillColor, borderColor, borderColorEnd);
    if (shape == nullptr) {
        shape = new shapes::RoundedOutlinedRect({0.0f, 0.0f}, p.width, p.height, p.borderRadius, p.borderWidth, fillColor, borderColor, borderColorEnd, 6);
        addShape(p, shape);
    }
    return shape;
}

}