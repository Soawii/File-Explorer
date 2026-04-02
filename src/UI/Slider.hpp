#pragma once
#include "UIElement.hpp"
#include <SFML/Graphics.hpp>
#include "StateVariable.hpp"
#include <functional>
#include "ProgressBar.hpp"

namespace ui {

class Slider : public UIElement {
public:
    Slider(float start, float min, float max, int z_index = -1, std::function<void()> onValueChanged = [](){});
    
    void update(MouseContext& mouseContext, KeyboardContext& keyboardContext) override;
    void checkChangedStates() override;

    float m_sliderPadding = 10.0f;
    float m_min, m_max;
    float m_var;
    std::function<void()> m_onValueChanged;
    UIElement *m_sliderPoint;
    ProgressBar *m_sliderLine;
private:
    void updateFillFromValue(float value);
    void updateThumbPosition();

    shapes::RoundedOutlinedRect fill_rect;
};

}