#include "TextElement.hpp"
#include "ShapePool.hpp"

namespace ui {

TextElement::TextElement(const sf::String& text, const sf::Font& font, unsigned int charSize, unsigned int thickness)
{
    m_string = text;

    m_text.setFont(font);
    m_text.setString(m_string);
    m_text.setCharacterSize(charSize);
    if (thickness != 0)
        m_text.setOutlineThickness(thickness);
    m_text.setOrigin(m_text.getLocalBounds().getPosition());
    m_context->sizeMode[0] = UISizeMode::FIT_CONTENT;
    m_context->sizeMode[1] = UISizeMode::FIT_CONTENT;

    for (int i = 0; i < sf::Mouse::ButtonCount; i++) {
        m_context->propogateMouse[i] = true;
    }
    for (int i = 0; i < sf::Keyboard::KeyCount; i++) {
        m_context->propogateKeyboard[i] = true;
    }
    m_context->propogateHover = true;
}

sf::Vector2f TextElement::getLocalBounds() {
    const Bounds& anchorRect = (m_parent == m_anchor 
        ? m_anchor->m_context->childrenBounds
        : m_anchor->m_context->anchorBounds);
    const sf::Vector2f absoluteSize = m_vars.size.absolute.get();
    const sf::Vector2f relativeSize = m_vars.size.relative.get();
    return {
        relativeSize.x * anchorRect.m_width + absoluteSize.x, 
        relativeSize.y * anchorRect.m_height + absoluteSize.y};
}

void TextElement::computeSize() {
    if (m_context->sizeMode[0] == UISizeMode::FIT_CONTENT || m_context->sizeMode[1] == UISizeMode::FIT_CONTENT) {
        const sf::Vector2f textLocalBounds = m_text.getLocalBounds().getSize();
        sf::Vector2f size = m_vars.size.absolute.get(), newSize = size;
        const float padding = m_vars.padding.get();
        if (m_context->sizeMode[0] == UISizeMode::FIT_CONTENT) {
            newSize.x = textLocalBounds.x + padding * 2;
        }
        if (m_context->sizeMode[1] == UISizeMode::FIT_CONTENT) {
            newSize.y = textLocalBounds.y + padding * 2;
        }
        if (fabs(size.x - newSize.x) > 0.1f || fabs(size.y - newSize.y) > 0.1f) {
            m_vars.size.absolute.setAll(newSize);
        }
    }
    computeBounds();

    bool recomputeBounds = false;
    const float padding = m_vars.padding.get();
    if (m_textOverflow == TextOverflow::WRAP) 
    {
        sf::String m_string_copy = m_string;
        const sf::Font* font = m_text.getFont();
        const size_t char_size = m_text.getCharacterSize();
        sf::Uint32 prev = 0;
        float totalWidth = 0.0f, widthSinceStart = 0.0f;
        const float maxWidth = 
            (m_context->sizeMode[0] == UISizeMode::FIXED ? 
            m_context->anchorBounds.m_width :
            m_parent->m_context->childrenBounds.m_width)
            - padding * 2 
            - std::abs(m_textPosAbs.x) + m_textOriginAbs.x;

        int start = -1;

        for (int i = 0; i < m_string_copy.getSize(); i++)
        {
            if (m_string_copy[i] == ' ') {
                if (totalWidth > maxWidth && start != -1) {
                    m_string_copy[start] = '\n';
                    start = i;
                    totalWidth = widthSinceStart;
                    widthSinceStart = 0.0f;
                    prev = 0;
                    continue;
                }
                start = i;
                widthSinceStart = 0.0f;
            }

            sf::Uint32 cur = static_cast<sf::Uint8>(m_string_copy[i]); // Assume ASCII; adapt if UTF-8.
            totalWidth += font->getKerning(prev, cur, char_size);
            const sf::Glyph& g = font->getGlyph(cur, char_size, false);
            totalWidth += g.advance;
            if (m_string_copy[i] != ' ')
                widthSinceStart += g.advance;

            if (i == int(m_string_copy.getSize()) - 1) {
                if (totalWidth > maxWidth && start != -1) {
                    m_string_copy[start] = '\n';
                    start = i;
                    totalWidth = widthSinceStart;
                    widthSinceStart = 0.0f;
                    prev = 0;
                    continue;
                }
                start = i;
            }
        }

        if (m_text.getString() != m_string_copy) {
            m_text.setString(m_string_copy);
            recomputeBounds = true;
        }
    }
    else if (m_textOverflow == TextOverflow::ELLIPSIS) {
        sf::String m_string_copy = m_string;
        const sf::Font* font = m_text.getFont();
        const size_t char_size = m_text.getCharacterSize();
        sf::Uint32 prev = 0;
        float width = 0.0f;
        const float maxWidth = 
            (m_context->sizeMode[0] == UISizeMode::FIXED ? 
            m_context->anchorBounds.m_width :
            m_parent->m_context->childrenBounds.m_width)
            - padding * 2 
            - std::abs(m_textPosAbs.x) + m_textOriginAbs.x;

        for (int i = 0; i < m_string_copy.getSize(); i++)
        {
            sf::Uint32 cur = static_cast<sf::Uint8>(m_string_copy[i]); // Assume ASCII; adapt if UTF-8.
            width += font->getKerning(prev, cur, char_size);
            const sf::Glyph& g = font->getGlyph(cur, char_size, false);
            width += g.advance;

            if (width > maxWidth) {
                int dot_amount = std::max(0, std::min(i - 1, 3));
                m_string_copy = m_string_copy.substring(0, i - dot_amount + 1) + std::string(dot_amount, '.');
                break;
            }
        }

        if (m_text.getString() != m_string_copy) {
            m_text.setString(m_string_copy);
            recomputeBounds = true;
        }
    }

    if (recomputeBounds) {
        if (m_context->sizeMode[0] == UISizeMode::FIT_CONTENT || m_context->sizeMode[1] == UISizeMode::FIT_CONTENT) {
            const sf::Vector2f textLocalBounds = m_text.getLocalBounds().getSize();
            sf::Vector2f size = m_vars.size.absolute.get(), newSize = size;
            const float padding = m_vars.padding.get();
            if (m_context->sizeMode[0] == UISizeMode::FIT_CONTENT) {
                newSize.x = textLocalBounds.x + padding * 2;
            }
            if (m_context->sizeMode[1] == UISizeMode::FIT_CONTENT) {
                newSize.y = textLocalBounds.y + padding * 2;
            }
            if (fabs(size.x - newSize.x) > 0.1f || fabs(size.y - newSize.y) > 0.1f) {
                m_vars.size.absolute.setAll(newSize);
            }
        }
        computeBounds();
    }
}

void TextElement::setString(const sf::String& str, bool computeSizeAfter) {
    if (m_string != str)
        m_context->dirtyBounds = true;
    m_string = str;
    m_text.setString(str);
    if (computeSizeAfter)
        computeSize();
}

void TextElement::setString(sf::String&& str, bool computeSizeAfter) {
    if (m_string != str)
        m_context->dirtyBounds = true;
    m_string = std::move(str);
    m_text.setString(m_string);
    if (computeSizeAfter)
        computeSize();
}

void TextElement::setFixedHeight(std::string possibleChars) {
    sf::Text text;
    text.setCharacterSize(m_text.getCharacterSize());
    text.setFont(*m_text.getFont());
    text.setString(possibleChars);
    const sf::FloatRect bounds = text.getLocalBounds();

    m_context->sizeMode[1] = UISizeMode::FIXED;
    m_vars.size.absolute.setAll({m_vars.size.absolute.get().x, bounds.getSize().y});
    m_textOriginAbs.y = bounds.getPosition().y;
}

void TextElement::draw(sf::RenderWindow *window, sf::IntRect& viewport) {
    const float top = m_context->anchorBounds.m_pos.y;
    const float left = m_context->anchorBounds.m_pos.x;
    const float bottom = m_context->anchorBounds.m_pos.y + m_context->anchorBounds.m_height;
    const float right = m_context->anchorBounds.m_pos.x + m_context->anchorBounds.m_width;
    if (bottom < viewport.top || 
            right < viewport.left ||
            left > viewport.left + viewport.width || 
            top > viewport.top + viewport.height) {
        return;
    }

    // ORIGIN
    sf::FloatRect bounds = m_text.getLocalBounds();
    if (!m_changeOrigin) {
        bounds.left = 0.0f;
        bounds.top = 0.0f;
    }
    sf::Vector2f originPos = bounds.getPosition();
    sf::Vector2f size = bounds.getSize();
    if (m_textOriginAbs.x != 0.0f) {
        originPos.x = m_textOriginAbs.x;
    }  
    if (m_textOriginAbs.y != 0.0f) {
        originPos.y = m_textOriginAbs.y;
    } 
    if (m_textOriginRel.x != 0.0f) {
        originPos.x += size.x * m_textOriginRel.x;
    }  
    if (m_textOriginRel.y != 0.0f) {
        originPos.y += size.y * m_textOriginRel.y;
    }
    const sf::Vector2f text_origin = m_text.getOrigin();
    if (fabs(text_origin.x - originPos.x) > 0.1f || fabs(text_origin.y - originPos.y) > 0.1f)
        m_text.setOrigin(int(originPos.x), int(originPos.y));

    // POSITION
    sf::Vector2f pos = m_context->anchorBounds.m_pos;
    pos.x = int(pos.x + m_textPosAbs.x + m_textPosRel.x * m_context->anchorBounds.m_width);
    pos.y = int(pos.y + m_textPosAbs.y + m_textPosRel.y * m_context->anchorBounds.m_height);

    // COLOR
    const Color colorHSL = m_vars.color.get();
    sf::Color color = hslToRgb(colorHSL);
    color.a *= m_context->calculatedOpacity;

    const float borderWidth = m_vars.borderWidth.get();
    if (fabs(m_text.getOutlineThickness() - borderWidth) > 0.1f) {
        m_text.setOutlineThickness(borderWidth);
    }
    if (borderWidth > 0.5f) {
        const Color borderColorHSL = m_vars.borderColor.get();
        sf::Color borderColor = hslToRgb(borderColorHSL);
        borderColor.a *= m_context->calculatedOpacity;
        m_text.setOutlineColor(borderColor);
    }

    if (color.a > 0) {
        const sf::Vector2f textPos = m_text.getPosition();
        if (fabs(textPos.x - pos.x) > 0.1f || fabs(textPos.y - pos.y) > 0.1f)
            m_text.setPosition(pos);
        m_text.setFillColor(color);
        window->draw(m_text, m_context->calculatedTransform);
    }
}

}