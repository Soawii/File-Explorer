#pragma once
#include "UIElement.hpp"
#include "../engine/EngineContext.hpp"

namespace ui {

class WindowElement : public UIElement {
public:
    WindowElement(EngineContext context);

    sf::FloatRect getGlobalBounds() override;
    void computeSize() override;
    bool checkDirtyBounds() override;
    void draw(sf::RenderWindow* window, sf::IntRect& viewport) override;
    void update(MouseContext& mouseContext, KeyboardContext& keyboardContext) override;

    EngineContext m_engineContext;
};

}