#pragma once
#include <SFML/Graphics.hpp>

namespace ui {

class MouseFrameContext {
public:
    MouseFrameContext();
    bool isPressed[sf::Mouse::ButtonCount];
    bool isConsumed[sf::Mouse::ButtonCount];
    bool isConsumedThisFrame[sf::Mouse::ButtonCount];
    bool hoverConsumed;
    sf::Vector2i pos;
    bool mouseWheelScrolled;
    int scrollDelta;
};

class MouseContext {
public:
    MouseContext();

    void press(sf::Mouse::Button button);
    void release(sf::Mouse::Button button);
    void consume(sf::Mouse::Button button);
    void consumeHover();
    void consumeScroll();

    bool isPressed(sf::Mouse::Button button);
    bool isPressedThisFrame(sf::Mouse::Button button);
    bool isPressedForMultipleFrames(sf::Mouse::Button button);
    bool isReleasedThisFrame(sf::Mouse::Button button);
    bool isConsumed(sf::Mouse::Button button);
    bool isConsumedThisFrame(sf::Mouse::Button button);

    MouseFrameContext m_prev, m_current;
};

}