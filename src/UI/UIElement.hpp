#pragma once
#include "MouseContext.hpp"
#include "KeyboardContext.hpp"
#include "UIElementContext.hpp"
#include "Shapes.hpp"
#include "Color.hpp"
#include "StateVariable.hpp"
#include "Bounds.hpp"
#include "UIAnimation.hpp"
#include "DrawBatchGroup.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <vector>

namespace ui {

class UIElement {
public:
    UIElement *m_parent = nullptr, *m_anchor = nullptr;
    std::vector<UIElement*>     m_children;
    std::vector<IUIAnimation*>  m_animations;
    UIElementContext*           m_context = new UIElementContext();
    StateVariables              m_vars;

    UIElement();
    ~UIElement();

    virtual sf::Vector2f getLocalBounds();
    virtual sf::FloatRect getGlobalBounds();

    virtual bool checkDirtyBounds();
    virtual void checkDirtyTransforms();

    virtual void computeBounds();
    virtual void computeSize();
    virtual void computeTransforms();

    virtual void propogateCall(const std::function<bool(UIElement*)>& func, bool toVisibleChildren = false);

    virtual void draw(sf::RenderWindow* window, sf::IntRect& viewport);
    virtual void update(MouseContext& mouseContext, KeyboardContext& keyboardContext);

    void setState(UIElementState state);
    virtual void checkChangedStates();

    void addChild(UIElement* child, UIElement* anchor = nullptr);
    UIElement* getChildById(std::string id);
    void setScrollPos(float pos, bool y_axis = true, bool smooth = false);
};

}