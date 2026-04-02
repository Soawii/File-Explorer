#pragma once
#include "UIElement.hpp"
#include <string>
#include <unordered_map>
#include "../engine/EngineContext.hpp"
#include "WindowElement.hpp"
#include <vector>
#include "Shapes.hpp"
#include "DrawBatchGroup.hpp"
#include "TextElement.hpp"

namespace ui {

class UIManager {
public:
    UIManager(EngineContext context);

    void addElement(UIElement* elem, UIElement* parent, UIElement* anchor = nullptr, std::string elementId = "", std::vector<std::string> elementClasses = {});
    void draw();
    void update();
    void handleEvent(sf::Event& e);

    UIElement* getElementById(std::string id);
    void registerClass(std::string newClass, std::function<void(UIElement*)> callback);
    void applyClass(UIElement* element, std::string element_class);

    void startFrame();
    void endFrame();

    void openModal(UIElement* modal);
    void closeModal();
    bool isModalOpen(UIElement* modal);

    size_t addError(sf::String error_text);
    void showError(size_t error_id, size_t milliseconds);

    WindowElement* m_windowElement;

    KeyboardContext m_keyboardContext;
    MouseContext m_mouseContext;
    EngineContext m_engineContext;

    std::vector<std::function<void(void)>> m_afterDirtyCheckCalls;

    UIElement* m_globalModal = nullptr;
    UIElement* m_modalBackdrop = nullptr;
    UIElement* m_globalError = nullptr;
private:
    std::unordered_map<std::string, std::function<void(UIElement*)>> m_classToCallback;
    std::unordered_map<std::string, UIElement*> m_idToElement;
    std::vector<shapes::Shape> m_endOfFrameDraws;
    std::vector<TextElement*> m_errorTextElements;
};

}