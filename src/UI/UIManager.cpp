#include "UIManager.hpp"
#include "WindowElement.hpp"
#include "Button.hpp"
#include "TextElement.hpp"
#include "../engine/EngineContext.hpp"
#include "ProgressBar.hpp"
#include "../engine/sync.hpp"
#include "Flex.hpp"
#include "Slider.hpp"
#include "TextInput.hpp"
#include "../util/util.hpp"
#include "UIAnimation.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <SFML/OpenGL.hpp>

namespace ui {

UIManager::UIManager(EngineContext context) : m_engineContext(context) {
    m_windowElement = new WindowElement(m_engineContext);
    addElement(m_windowElement, nullptr, nullptr, "window", {});

    m_windowElement->m_context->minSizePasses = 2;
    m_windowElement->m_context->minTransformPasses = 2;
}

void UIManager::addElement( UIElement* elem, UIElement* parent, UIElement* anchor, 
                            std::string elementId, std::vector<std::string> elementClasses) {
    if (anchor == nullptr)
        anchor = parent;

    if (elementId != "") {
        if (m_idToElement.find(elementId) != m_idToElement.end()) {
            std::cerr << "Duplicate ID: " << elementId << std::endl;
            return;
        }
        m_idToElement[elementId] = elem;
    }

    elem->m_anchor = anchor;
    elem->m_parent = parent;
    elem->m_context->id = elementId;

    if (parent != nullptr) {
        parent->m_children.push_back(elem);
        elem->m_context->z_index = parent->m_context->z_index;
    }

    for (int i = 0; i < elementClasses.size(); i++) {
        applyClass(elem, elementClasses[i]);
    }
}

void UIManager::draw() {
    glClear(GL_STENCIL_BUFFER_BIT);

    std::vector<UIElement*> drawables;
    for (UIElement* c : m_windowElement->m_children) {
        c->propogateCall([&drawables](UIElement* e) {
            if (e->m_context->m_current.state == UIElementState::HIDDEN || e->m_context->calculatedOpacity < 0.001f) {
                return false;
            }
            drawables.push_back(e);
            return !(e->m_context->overflow[0] == UIOverflowMode::HIDDEN ||
                e->m_context->overflow[1] == UIOverflowMode::HIDDEN ||
                e->m_context->overflow[0] == UIOverflowMode::SCROLL || 
                e->m_context->overflow[1] == UIOverflowMode::SCROLL || 
                e->m_context->dontDrawChildren);
        }, true);
    }
    std::stable_sort(drawables.begin(), drawables.end(), [](UIElement* l, UIElement* r) {
        return l->m_context->z_index < r->m_context->z_index;
    });
    sf::IntRect window_viewport = m_engineContext.m_window->getViewport(m_engineContext.m_window->getView());
    for (int i = 0; i < drawables.size(); i++) {
        drawables[i]->draw(m_engineContext.m_window, window_viewport);
    }

    const sf::RenderStates states = sf::RenderStates();
    for (int i = 0; i < m_endOfFrameDraws.size(); i++) {
        m_endOfFrameDraws[i].draw(*m_engineContext.m_window, states);
    }
    m_endOfFrameDraws.clear();
}

void UIManager::update() {
    m_mouseContext.m_current.pos = sf::Mouse::getPosition(*m_engineContext.m_window);

    std::vector<UIElement*> updateables;

    m_windowElement->propogateCall([&updateables](UIElement* e) {
        if (e->m_context->m_current.state == UIElementState::HIDDEN || e->m_context->m_current.state == UIElementState::DISABLED) {
            e->m_context->mousePressedHere = false;
            return false;
        }
        if (!e->m_context->dontUpdateYourself)
            updateables.push_back(e);
        return !e->m_context->dontUpdateChildren;
    }, true);
    std::reverse(updateables.begin(), updateables.end());
    std::stable_sort(updateables.begin(), updateables.end(), [](UIElement* l, UIElement* r) {
        return l->m_context->z_index > r->m_context->z_index;
    });

    for (int i = 0; i < updateables.size(); i++) {
        updateables[i]->update(m_mouseContext, m_keyboardContext);
    }

    m_windowElement->propogateCall([](UIElement* e){
        if (e->m_context->m_current.state == UIElementState::DISABLED)
            return false;
        if (!e->m_context->statelessElement)
            e->checkChangedStates();
        return !e->m_context->statelessChildren;
    }, true);

    m_windowElement->checkDirtyBounds();
    m_windowElement->checkDirtyTransforms();

    for (int i = 0; i < m_afterDirtyCheckCalls.size(); i++) {
        m_afterDirtyCheckCalls[i]();
    }

    m_windowElement->propogateCall([](UIElement* e) {
        if ((e->m_context->dirtyBounds || e->m_context->minSizePasses > 0) && !e->m_context->dontComputeSizeOfYourself) {
            if (e->m_context->minSizePasses > 0) {
                e->m_context->minSizePasses--;
            }
            e->computeSize();
            return false;
        }
        return !e->m_context->dontComputeSizeOfChildren;
    });
    m_windowElement->propogateCall([](UIElement* e) {
        if ((e->m_context->dirtyBounds || e->m_context->dirtyTransform || e->m_context->minTransformPasses > 0) && !e->m_context->dontComputeSizeOfYourself) {
            if (e->m_context->minTransformPasses > 0) {
                e->m_context->minTransformPasses--;
            }
            e->computeTransforms();
            return false;
        }
        return !e->m_context->dontComputeSizeOfChildren;
    });
}

void UIManager::handleEvent(sf::Event& e) {
    if (e.type == sf::Event::KeyPressed) {
        m_keyboardContext.press(e.key.code);
    }
    else if (e.type == sf::Event::KeyReleased) {
        m_keyboardContext.release(e.key.code);
    }
    else if (e.type == sf::Event::TextEntered)
    {
        sf::Uint32 c = e.text.unicode;
        m_keyboardContext.m_current.textEnteredThisFrame.push_back(c);
    }
    else if (e.type == sf::Event::MouseButtonPressed) {
        m_mouseContext.press(e.mouseButton.button);
    }
    else if (e.type == sf::Event::MouseButtonReleased) {
        m_mouseContext.release(e.mouseButton.button);
    }
    else if (e.type == sf::Event::MouseWheelScrolled) {
        m_mouseContext.m_current.mouseWheelScrolled = true;
        m_mouseContext.m_current.scrollDelta = e.mouseWheelScroll.delta;
    }
}

UIElement* UIManager::getElementById(std::string id) {
    if (m_idToElement.find(id) != m_idToElement.end())
        return m_idToElement[id];
    return nullptr;
}

void UIManager::registerClass(std::string newClass, std::function<void(UIElement*)> callback) {
    m_classToCallback[newClass] = callback;
}

void UIManager::startFrame() {
    m_windowElement->propogateCall([](UIElement* e) {
        e->m_context->startFrame();
        return !e->m_context->dontComputeSizeOfChildren || !e->m_context->statelessChildren;
    }, !conf::settings::smoothSort);

    for (int i = 0; i < sf::Keyboard::KeyCount; i++) {
        m_keyboardContext.m_current.isTriggeredThisFrame[i] = false;
        m_keyboardContext.m_current.isConsumedThisFrame[i] = false;
    }
    m_keyboardContext.m_current.textEnteredThisFrame.clear();

    m_mouseContext.m_current.mouseWheelScrolled = false;
    m_mouseContext.m_current.hoverConsumed = false;
    for (int i = 0; i < sf::Mouse::ButtonCount; i++) {
        m_mouseContext.m_current.isConsumedThisFrame[i] = false;
    }
}
void UIManager::endFrame() {
    m_windowElement->propogateCall([](UIElement* e) {
        e->m_context->endFrame();
        return true;
    }, true);

    m_keyboardContext.m_prev = m_keyboardContext.m_current;
    m_mouseContext.m_prev = m_mouseContext.m_current;
}

void UIManager::applyClass(UIElement* element, std::string element_class) {
    if (m_classToCallback.find(element_class) != m_classToCallback.end()) {
        m_classToCallback[element_class](element);
    }
    else {
        std::cout << "Unknown class when trying to initialize id [" << element->m_context->id << "]: " << element_class << '\n';
    }
}

void UIManager::openModal(UIElement* modal) {
    if (m_globalModal != nullptr)
        return;

    m_globalModal = modal;
    m_globalModal->setState(UIElementState::NORMAL);
    m_modalBackdrop->setState(UIElementState::NORMAL);
}

void UIManager::closeModal() {
    if (m_globalModal == nullptr)
        return;

    m_globalModal->setState(UIElementState::DISABLED);
    m_globalModal = nullptr;
    m_modalBackdrop->setState(UIElementState::DISABLED);
}

bool UIManager::isModalOpen(UIElement* modal) {
    return m_globalModal == modal;
}

size_t UIManager::addError(sf::String error_text) {
    if (m_globalError == nullptr)
        return 0;
    TextElement* error_text_element = new TextElement(error_text, conf::fonts::mono_r, 16);
    addElement(error_text_element, m_globalError, m_globalError, error_text.toAnsiString(), {});
    error_text_element->m_vars.color.setAll(Color(255,255,255));
    error_text_element->m_vars.opacity.setState(UIElementState::DISABLED, 0.0f);
    error_text_element->m_textOverflow = TextOverflow::WRAP;
    m_errorTextElements.push_back(error_text_element);
    return m_errorTextElements.size() - 1;
}

void UIManager::showError(size_t error_id, size_t milliseconds) {
    if (m_globalError == nullptr)
        return;

    for (int i = 0; i < m_errorTextElements.size(); i++) {
        if (i == error_id)
            continue;
        if (m_errorTextElements[i]->m_context->m_current.state != UIElementState::DISABLED)
            m_errorTextElements[i]->setState(UIElementState::DISABLED);
    }
    m_errorTextElements[error_id]->setState(UIElementState::NORMAL);

    if (m_globalError->m_context->callbacksOnTimepoint.size() > 0)
        m_globalError->m_context->callbacksOnTimepoint.clear();

    m_globalError->m_context->callbacksOnTimepoint.push_back(std::make_pair(
        conf::time::now + std::chrono::milliseconds(milliseconds),
        [this]() {
            m_globalError->setState(UIElementState::DISABLED);
        }
    ));

    if (m_globalError->m_context->m_current.state == UIElementState::DISABLED) {
        m_globalError->setState(UIElementState::NORMAL);
    }
    m_globalError->m_context->dirtyBounds = true;
}

}