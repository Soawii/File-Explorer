#include "TextInput.hpp"
#include <SFML/OpenGL.hpp>
#include "ShapePool.hpp"

namespace ui {

TextInput::TextInput(sf::String startingValue, sf::String allowedChars, sf::String fixedHeightChars, sf::String placeholder, sf::Font& font, unsigned int fontSize, unsigned int thickness) 
: m_cursorX(0.0f, 0.0f, TimingFunctions::smoothstep) 
{
    m_context->sizeMode[1] = UISizeMode::FIT_CONTENT;
    m_context->focusable = true;
    m_context->dontUpdateChildren = true;
    m_context->statelessChildren = true;

    m_onFocus.push_back([this]() {
        m_placeholderText->m_vars.opacity.setAll(0.0f);
    });
    m_context->onUnfocus.push_back([this]() {
        if (m_text->m_text.getString() == "") {
            m_placeholderText->m_vars.opacity.setAll(1.0f);
        }
    });

    m_context->onPress[0].push_back([this](MouseContext& m) {
        if (!m_context->focused) {
            m_context->focused = true;
            m_context->triggerEvents(m_onFocus);
            moveCursor(0);
        }
    });

    m_text = new TextElement(startingValue, font, fontSize, thickness);
    m_text->setFixedHeight(fixedHeightChars);
    addChild(m_text);
    m_text->m_vars.color.setAll(Color(40,40,40));

    m_placeholderText = new TextElement(placeholder, font, fontSize, thickness);
    m_placeholderText->setFixedHeight(fixedHeightChars);
    addChild(m_placeholderText);
    m_placeholderText->m_vars.color.setAll(Color(200,200,200));

    // cursor draw
    m_context->endOfDraw.push_back([this](sf::RenderWindow* window) {
        if (m_context->focused && int(std::chrono::duration<float>(conf::time::now - m_lastCursorMoveTime).count() / m_cursorBlinkTime) % 2 == 0) {
            const RoundedRectParams p(2.0f, m_text->m_text.getCharacterSize() + 8.0f, 0.0f, 0);
            const shapes::RoundedRect* cursor = ShapePool::findOrCreateRoundedRect(p, sf::Color(90,90,95));
            sf::Transform calculatedTransform = m_context->calculatedTransform;
            calculatedTransform.translate({m_context->childrenBounds.m_pos.x, m_context->childrenBounds.m_pos.y + m_context->childrenBounds.m_height / 2.0f - p.height / 2.0f});
            calculatedTransform.translate((m_align == TextAlign::START ? m_cursorX.get() : m_context->childrenBounds.m_width - m_cursorX.get()) - m_context->scrollPosX.get(), 0.0f);
            cursor->draw(*window, calculatedTransform);
        }
    });

    if (allowedChars.getSize() == 0) {
        std::u32string allowedUTF32 = 
            U"qwertyuiopasdfghjklzxcvbnm"
            U"QWERTYUIOPASDFGHJKLZXCVBNM"
            U"12345678890"
            U"йцукенгшщщзхъфывапроллджэячсмитьбю"
            U"ЙЦУКЕНГШЩЗХЪХФЫВАПРОЛДЖЭЯЧСМИТЬБЮ"
            U",.- _:/\\";
        allowedChars = sf::String::fromUtf32(allowedUTF32.begin(), allowedUTF32.end());
    }

    const std::string wordBoundaries = "., ";
    
    for (int i = 0; i < allowedChars.getSize(); i++) {
        sf::Uint32 c = allowedChars[i];
        m_context->onTextEnter[c] = [this, c]() {
            insert(c);
        };
    }
    m_context->onTriggerKeys.push_back({Keypress(sf::Keyboard::Left, false, false, true, false), [this]() {
        this->moveCursor(std::min(this->m_text->m_text.getString().getSize(), this->m_cursorPos + 1));
    }});
    m_context->onTriggerKeys.push_back({Keypress(sf::Keyboard::Right, false, false, true, false), [this]() {
        this->moveCursor(std::max(0, int(this->m_cursorPos) - 1));
    }});
    m_context->onTriggerKeys.push_back({Keypress(sf::Keyboard::Left, false, false, true, true), [this, wordBoundaries]() {
        const sf::String s = m_text->m_string;
        const size_t size = s.getSize();
        int word_boundary = std::min(this->m_cursorPos + 1, size);
        while (word_boundary < size) {
            if (s[size - word_boundary - 1] != ' ')
                break;
            word_boundary++;
        }
        while (word_boundary < size) {
            if (wordBoundaries.find_first_of(std::min(255, (int)s[size - word_boundary - 1])) != std::string::npos) 
                break;
            word_boundary++;
        }
        this->moveCursor(std::min(int(size), word_boundary));
    }});
    m_context->onTriggerKeys.push_back({Keypress(sf::Keyboard::Right, false, false, true, true), [this, wordBoundaries]() {
        const sf::String s = m_text->m_string;
        int word_boundary = std::max(int(this->m_cursorPos) - 1, 0);
        while (word_boundary > 0) {
            if (s[s.getSize() - word_boundary] != ' ')
                break;
            word_boundary--;
        }
        while (word_boundary > 0) {
            if (wordBoundaries.find_first_of(s[s.getSize() - word_boundary]) != std::string::npos) 
                break;
            word_boundary--;
        }
        this->moveCursor(std::max(0, word_boundary));
    }});


    m_context->onTriggerKeys.push_back({Keypress(sf::Keyboard::Backspace, false, false, true, false), [this]() {
        this->backspaceAt(this->m_cursorPos, 1);
    }});
    m_context->onTriggerKeys.push_back({Keypress(sf::Keyboard::Delete, false, false, true, false), [this]() {
        this->deleteAt(this->m_cursorPos, 1);
    }});

    m_context->onTriggerKeys.push_back({Keypress(sf::Keyboard::Backspace, false, false, true, true), [this, wordBoundaries]() {
        const sf::String s = m_text->m_string;
        int word_boundary = std::min(this->m_cursorPos + 1, s.getSize());
        while (word_boundary < s.getSize()) {
            if (s[s.getSize() - word_boundary - 1] != ' ')
                break;
            word_boundary++;
        }
        while (word_boundary < s.getSize()) {
            if (wordBoundaries.find_first_of(s[s.getSize() - word_boundary - 1]) != std::string::npos) 
                break;
            word_boundary++;
        }
        this->backspaceAt(this->m_cursorPos, word_boundary - this->m_cursorPos);
    }});

    m_context->onTriggerKeys.push_back({Keypress(sf::Keyboard::Delete, false, false, true, true), [this, wordBoundaries]() {
        const sf::String s = m_text->m_string;
        int word_boundary = std::max(int(this->m_cursorPos) - 1, 0);
        while (word_boundary > 0) {
            if (s[s.getSize() - word_boundary] != ' ')
                break;
            word_boundary--;
        }
        while (word_boundary > 0) {
            if (wordBoundaries.find_first_of(s[s.getSize() - word_boundary]) != std::string::npos) 
                break;
            word_boundary--;
        }
        this->deleteAt(this->m_cursorPos, this->m_cursorPos - word_boundary);
    }});

    setAlign(TextAlign::START);
}

void TextInput::onInput() {
    if (m_context->m_current.state == UIElementState::FOCUSED || m_text->m_text.getString() != "") {
        m_placeholderText->m_vars.opacity.setAll(0.0f);
    }
    else {
        m_placeholderText->m_vars.opacity.setAll(1.0f);
    }
    m_context->dirtyBounds = true;
    m_context->triggerEvents(m_onInput);
}

void TextInput::setString(const sf::String& s) {
    m_text->setString(s);
    onInput();
    moveCursor(0);
}

void TextInput::insert(sf::String s) {
    sf::String currText = m_text->m_string;
    if (m_maxChars < s.getSize() + currText.getSize()) {
        return;
    }
    currText.insert(currText.getSize() - m_cursorPos, s);
    m_text->setString(currText);
    onInput();
    moveCursor(m_cursorPos);
}

void TextInput::insertAt(sf::String s, size_t pos) {
    sf::String currText = m_text->m_string;
    if (m_maxChars < s.getSize() + currText.getSize()) {
        return;
    }
    currText.insert(currText.getSize() - pos, s);
    m_text->setString(currText);
    onInput();
    moveCursor(pos);
}
void TextInput::backspaceAt(size_t pos, size_t n) {
    sf::String currText = m_text->m_string;
    const size_t size = currText.getSize();
    if (pos == size)
        return;
    currText = currText.substring(0, size - pos - n) + currText.substring(size - pos);
    m_text->setString(currText);
    onInput();
    moveCursor(pos);
}
void TextInput::deleteAt(size_t pos, size_t n) {
    if (pos == 0)
        return;
    sf::String currText = m_text->m_string;
    const size_t size = currText.getSize();
    currText = currText.substring(0, size - pos) + currText.substring(size - pos + n);
    m_text->setString(currText);
    onInput();
    moveCursor(pos - n);
}
void TextInput::moveCursor(size_t newPos) {
    sf::String text = m_text->m_string;
    newPos = std::max(size_t(0), std::min(text.getSize(), newPos));
    m_cursorPos = newPos;

    sf::Text measureText;
    measureText.setFont(*m_text->m_text.getFont());
    measureText.setCharacterSize(m_text->m_text.getCharacterSize());
    sf::String measureString;
    if (m_align == TextAlign::START) {
        measureString = text.substring(0, text.getSize() - m_cursorPos);
    }
    else if (m_align == TextAlign::END) {
        measureString = text.substring(text.getSize() - m_cursorPos);
    }
    measureText.setString(measureString);
    m_cursorX = measureText.getLocalBounds().width;
    m_lastCursorMoveTime = conf::time::now;
}

void TextInput::setAlign(TextAlign newAlign) {
    m_align = newAlign;
    if (newAlign == TextAlign::START) {
        m_text->m_vars.origin.relative.setAll({0.0f, 0.5f});
        m_text->m_vars.pos.relative.setAll({0.0f, 0.5f});
        m_placeholderText->m_vars.origin.relative.setAll({0.0f, 0.5f});
        m_placeholderText->m_vars.pos.relative.setAll({0.0f, 0.5f});
    }
    else if (newAlign == TextAlign::END) {
        m_text->m_vars.origin.relative.setAll({1.0f, 0.5f});
        m_text->m_vars.pos.relative.setAll({1.0f, 0.5f});
        m_placeholderText->m_vars.origin.relative.setAll({1.0f, 0.5f});
        m_placeholderText->m_vars.pos.relative.setAll({1.0f, 0.5f});
    }
    moveCursor(0);
    onInput();
}

}