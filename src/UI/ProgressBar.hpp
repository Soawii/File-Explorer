#pragma once
#include "UIElement.hpp"
#include <SFML/Graphics.hpp>
#include "StateVariable.hpp"
#include <functional>

namespace ui {

class ProgressBar : public UIElement {
public:
    ProgressBar(std::function<float(void)>* getFillRatio = nullptr);

    void draw(sf::RenderWindow* window, sf::IntRect& viewport) override;
    void checkChangedStates() override;

    TimingVariable<float> m_fillRatio;
    std::function<float(void)>* m_getFillRatio;
    ContextStateVariable<Color> m_fillColor;
};

}