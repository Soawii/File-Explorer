#pragma once
#include <SFML/Graphics.hpp>

namespace ui {
    class KeyboardFrameContext {
    public:
        KeyboardFrameContext();
        bool isPressed[sf::Keyboard::KeyCount];
        bool isTriggeredThisFrame[sf::Keyboard::KeyCount];
        bool isConsumed[sf::Keyboard::KeyCount];
        bool isConsumedThisFrame[sf::Keyboard::KeyCount];
        std::vector<sf::Uint32> textEnteredThisFrame;
    };

    class KeyboardContext {
    public:
        KeyboardContext();

        void press(sf::Keyboard::Key key);
        void release(sf::Keyboard::Key key);
        void consume(sf::Keyboard::Key key);

        bool isPressed(sf::Keyboard::Key key);
        bool isPressedThisFrame(sf::Keyboard::Key key);
        bool isTriggeredThisFrame(sf::Keyboard::Key key);
        bool isReleasedThisFrame(sf::Keyboard::Key key);
        bool isConsumed(sf::Keyboard::Key key);
        bool isConsumedThisFrame(sf::Keyboard::Key key);

        KeyboardFrameContext m_prev, m_current;
    };
}