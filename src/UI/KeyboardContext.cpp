#include "KeyboardContext.hpp"

namespace ui {

KeyboardFrameContext::KeyboardFrameContext() {
    for (int i = 0; i < sf::Keyboard::KeyCount; i++) {
        isPressed[i] = false;
        isTriggeredThisFrame[i] = false;
        isConsumed[i] = false;
        isConsumedThisFrame[i] = false;
    }
}


KeyboardContext::KeyboardContext() {

}

void KeyboardContext::press(sf::Keyboard::Key key) {
    m_current.isPressed[key] = true;
    m_current.isTriggeredThisFrame[key] = true;
}
void KeyboardContext::release(sf::Keyboard::Key key) {
    m_current.isPressed[key] = false;
}
void KeyboardContext::consume(sf::Keyboard::Key key) {
    m_current.isConsumed[key] = true;
    m_current.isConsumedThisFrame[key] = true;
}

bool KeyboardContext::isPressed(sf::Keyboard::Key key) {
    return m_current.isPressed[key];
}
bool KeyboardContext::isPressedThisFrame(sf::Keyboard::Key key) {
    return m_current.isPressed[key] && !m_prev.isPressed[key];
}
bool KeyboardContext::isTriggeredThisFrame(sf::Keyboard::Key key) {
    return m_current.isTriggeredThisFrame[key];
}
bool KeyboardContext::isReleasedThisFrame(sf::Keyboard::Key key) {
    return !m_current.isPressed[key] && m_prev.isPressed[key];
}
bool KeyboardContext::isConsumed(sf::Keyboard::Key key) {
    return m_current.isConsumed[key];
}
bool KeyboardContext::isConsumedThisFrame(sf::Keyboard::Key key) {
    return m_current.isConsumedThisFrame[key];
}

}