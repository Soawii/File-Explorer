#pragma once
#include "TextElement.hpp"
#include <string>
#include "../util/TimingVariable.hpp"

namespace ui {

enum class TextAlign {
    START = 0,
    END
};

class TextInput : public UIElement {
public:
    TextInput(sf::String startingValue, sf::String allowedChars, sf::String fixedHeightChars, sf::String placeholder, sf::Font& font, unsigned int fontSize, unsigned int thickness);

    void onInput();
    void setString(const sf::String& s);
    void insert(sf::String s);
    void insertAt(sf::String s, size_t pos);
    void backspaceAt(size_t pos, size_t n);
    void deleteAt(size_t pos, size_t n);
    void moveCursor(size_t newPos);
    void setAlign(TextAlign newAlign);

    TimingVariable<float> m_cursorX;
    std::chrono::steady_clock::time_point m_lastCursorMoveTime;
    std::vector<std::function<void(void)>> m_onInput;
    std::vector<std::function<void(void)>> m_onFocus;

    TextElement *m_text, *m_placeholderText;

    TextAlign m_align = TextAlign::START;
    size_t m_cursorPos = -1;
    float m_cursorBlinkTime = 0.5f;
    size_t m_maxChars = 100000;
};

}