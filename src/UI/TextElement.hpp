#pragma once
#include "UIElement.hpp"
#include <SFML/Graphics.hpp>
#include <string>

namespace ui {

enum class TextOverflow {
    VISIBLE,
    WRAP,
    ELLIPSIS
};

class TextElement : public UIElement {
public:
    TextElement(const sf::String& text, const sf::Font& font, unsigned int charSize = 24, unsigned int thickness = 0);

    sf::Vector2f getLocalBounds() override;
    void computeSize() override;

    void setString(const sf::String& str, bool computeSizeAfter = true);
    void setString(sf::String&& str, bool computeSizeAfter = true);

    void setFixedHeight(std::string possibleChars);
    void setFillColor(const sf::Color& color);
    void draw(sf::RenderWindow* window, sf::IntRect& viewport) override;

    sf::String m_string;
    sf::Text m_text;
    TextOverflow m_textOverflow = TextOverflow::VISIBLE;
    sf::Vector2f m_textOriginAbs{0.0f,0.0f}, m_textOriginRel{0.0f,0.0f};
    sf::Vector2f m_textPosAbs{0.0f,0.0f}, m_textPosRel{0.0f,0.0f};
    bool m_changeOrigin = true;
};

}