#include "MouseContext.hpp"

namespace ui {

MouseFrameContext::MouseFrameContext() {
    for (int i = 0; i < sf::Mouse::ButtonCount; i++) {
        isPressed[i] = false;
        isConsumed[i] = false;
        isConsumedThisFrame[i] = false;
    }
    pos = {0, 0};
    mouseWheelScrolled = false;
    scrollDelta = 0;
    hoverConsumed = false;
}

MouseContext::MouseContext() {}

void MouseContext::press(sf::Mouse::Button button) {
    m_current.isPressed[button] = true;
}
void MouseContext::release(sf::Mouse::Button button) {
    m_current.isPressed[button] = false;
}
void MouseContext::consume(sf::Mouse::Button button) {
    m_current.isConsumed[button] = true;
    m_current.isConsumedThisFrame[button] = true;
}
void MouseContext::consumeHover() {
    m_current.hoverConsumed = true;
}
void MouseContext::consumeScroll() {
    m_current.mouseWheelScrolled = false;
}
bool MouseContext::isPressed(sf::Mouse::Button button) {
    return m_current.isPressed[button];
}
bool MouseContext::isPressedThisFrame(sf::Mouse::Button button) {
    return m_current.isPressed[button] && !m_prev.isPressed[button];
}
bool MouseContext::isPressedForMultipleFrames(sf::Mouse::Button button) {
    return m_current.isPressed[button] && m_prev.isPressed[button];
}
bool MouseContext::isReleasedThisFrame(sf::Mouse::Button button) {
    return !m_current.isPressed[button] && m_prev.isPressed[button];
}
bool MouseContext::isConsumed(sf::Mouse::Button button) {
    return m_current.isConsumed[button];
}
bool MouseContext::isConsumedThisFrame(sf::Mouse::Button button) {
    return m_current.isConsumedThisFrame[button];
}

}