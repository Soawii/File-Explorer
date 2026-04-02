#pragma once
#include "Shapes.hpp"
#include <vector>
#include <unordered_map>

namespace ui {

struct RoundedRectParams {
    size_t width, height, borderRadius, quarterPoints;
    RoundedRectParams(float width_, float height_, float borderRadius_, size_t quarterPoints_);
    bool operator==(const RoundedRectParams& o) const;
};    
struct RoundedRectParamsHasher {
    size_t operator()(const RoundedRectParams& s) const;
};

struct RoundedOutlinedRectParams {
    size_t width, height, borderWidth, borderRadius, quarterPoints;
    RoundedOutlinedRectParams(float width_, float height_, float borderWidth_, float borderRadius_, size_t quarterPoints_);
    bool operator==(const RoundedOutlinedRectParams& o) const;
};    
struct RoundedOutlinedRectParamsHasher {
    size_t operator()(const RoundedOutlinedRectParams& s) const;
};

class ShapePool {
public:     
    static std::unordered_map<RoundedRectParams, shapes::RoundedRect*, RoundedRectParamsHasher> m_roundedRects;
    static std::unordered_map<RoundedOutlinedRectParams, shapes::RoundedOutlinedRect*, RoundedOutlinedRectParamsHasher> m_roundedOutlinedRects;

    static void addShape(const RoundedRectParams& p, shapes::RoundedRect* shape);
    static void addShape(const RoundedOutlinedRectParams& p, shapes::RoundedOutlinedRect* shape);

    static shapes::RoundedRect* findRoundedRect(const RoundedRectParams& p, const sf::Color& fillColor);
    static shapes::RoundedRect* findOrCreateRoundedRect(const RoundedRectParams& p, const sf::Color& fillColor);
    static shapes::RoundedOutlinedRect* findRoundedOutlinedRect(const RoundedOutlinedRectParams& p, const sf::Color& fillColor, const sf::Color& borderColor);
    static shapes::RoundedOutlinedRect* findRoundedOutlinedRect(const RoundedOutlinedRectParams& p, const sf::Color& fillColor, const sf::Color& borderColor, const sf::Color& borderColorEnd);
    static shapes::RoundedOutlinedRect* findOrCreateRoundedOutlinedRect(const RoundedOutlinedRectParams& p, const sf::Color& fillColor, const sf::Color& borderColor);
    static shapes::RoundedOutlinedRect* findOrCreateRoundedOutlinedRect(const RoundedOutlinedRectParams& p, const sf::Color& fillColor, const sf::Color& borderColor, const sf::Color& borderColorEnd);
};

}