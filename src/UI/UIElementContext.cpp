#include "UIElementContext.hpp"
#include <iostream>

namespace ui {

UIElementContext::UIElementContext()
:   scrollPosX(0.0f, 0.0f, TimingFunctions::easeOutQuad),
    scrollPosY(0.0f, 0.0f, TimingFunctions::easeOutQuad)
{
    for (int i = 0; i < sf::Mouse::ButtonCount; i++) {
        propogateMouse[i] = false;
    }
    for (int i = 0; i < sf::Keyboard::KeyCount; i++) {
        propogateKeyboard[i] = false;
    }
}

bool UIElementContext::didMouseMoveIn() {
    return m_prev.state == UIElementState::NORMAL && m_current.state == UIElementState::HOVER;
}
bool UIElementContext::didMouseMoveOut() {
    return m_prev.state == UIElementState::HOVER && m_current.state == UIElementState::NORMAL;
}
void UIElementContext::startFrame() {
    m_prev = m_current;
    dirtyBounds = false;
    dirtyTransform = false;
}
void UIElementContext::endFrame() {
}
bool UIElementContext::wasStateChanged() {
    return m_prev.state != m_current.state;
}
void UIElementContext::triggerEvents(const std::vector<std::function<void()>>& events) {
    for (int i = 0; i < events.size(); i++) {
        events[i]();
    }
}
void UIElementContext::triggerEvents(const std::vector<std::function<void(MouseContext&)>>& events, MouseContext& context) {
    for (int i = 0; i < events.size(); i++) {
        events[i](context);
    }
}
void UIElementContext::triggerEvents(const std::vector<std::function<void(MouseContext&)>>& events, MouseContext&& context) {
    for (int i = 0; i < events.size(); i++) {
        events[i](context);
    }
}

}