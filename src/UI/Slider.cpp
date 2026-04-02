#include "Slider.hpp"
#include "ShapePool.hpp"
#include "ProgressBar.hpp"

namespace ui {

Slider::Slider(float start, float min, float max, int z_index, std::function<void()> onValueChanged) 
: m_min(min), m_max(max), m_var(start),
    m_onValueChanged(onValueChanged),
    fill_rect({0.0f, 0.0f}, 1.0f, 1.0f, 0.0f)
{
    if (z_index != -1) {
        m_context->z_index = z_index;
    }

    m_sliderLine = new ProgressBar();
    addChild(m_sliderLine);
    m_sliderLine->m_vars.size.relative.setAll({1.0f, 0.0f});
    m_sliderLine->m_vars.size.absolute.setAll({0.0f, 8.0f});
    m_sliderLine->m_vars.borderRadius.setAll(100.0f);
    m_sliderLine->m_vars.color.setAll(Color(0,0,255));
    m_sliderLine->m_vars.origin.relative.setAll({0.0f, 0.5f});
    m_sliderLine->m_vars.pos.relative.setAll({0.0f, 0.5f});
    m_sliderLine->m_context->dontUpdateYourself = true;

    m_sliderPoint = new UIElement();
    addChild(m_sliderPoint);
    m_sliderPadding = 10.0f;
    m_sliderPoint->m_vars.size.absolute.setAll({20.0f, 20.0f});
    m_sliderPoint->m_vars.borderRadius.setAll(100.0f);
    m_sliderPoint->m_vars.color.setAll(Color(245,0,0));
    m_sliderPoint->m_vars.origin.relative.setAll({0.5f, 0.5f});
    m_sliderPoint->m_vars.pos.relative.setAll({0.0f, 0.5f});
    m_sliderPoint->m_context->dontUpdateYourself = true;

    m_vars.color.setAll(Color(0,0,0,0));
    m_vars.borderColor.setAll(Color(0,0,0,0));
    m_context->sizeMode[1] = UISizeMode::FIT_CONTENT;

    m_context->fitContentFixed = true;
    m_context->copyTransformToChildren = true;

    updateFillFromValue(m_var);
}


void Slider::update(MouseContext& mouseContext, KeyboardContext& keyboardContext) {
    if (m_context->m_current.state == UIElementState::HIDDEN || m_context->m_current.state == UIElementState::DISABLED) {
        m_context->mousePressedHere = false;
        return;
    }
    UIElement::update(mouseContext, keyboardContext);

    if (m_context->mousePressedHere) {
        const Bounds& bounds = m_context->childrenBounds;

        const sf::Vector2f mouseWorld(mouseContext.m_current.pos);
        const sf::Transform inverse = m_context->mouseBounds.m_transform.getInverse();
        const sf::Vector2f mouseLocal = inverse.transformPoint(mouseWorld);

        float clampedX = std::max(bounds.m_pos.x + m_sliderPadding, std::min(mouseLocal.x, bounds.m_pos.x + bounds.m_width - m_sliderPadding));
        float ratio = (clampedX - bounds.m_pos.x - m_sliderPadding) / (bounds.m_width - 2.0f * m_sliderPadding);
        ratio = std::max(0.0f, std::min(ratio, 1.0f));

        const float newValue = float(m_min + ratio * (m_max - m_min));
        if (newValue != m_var) {
            m_var = newValue;
            m_onValueChanged();
        }

        m_sliderLine->m_fillRatio = ratio;
        updateThumbPosition();
    }
}

void Slider::checkChangedStates() {
    UIElement::checkChangedStates();
    m_sliderLine->m_fillColor.checkChangedState();
}

void Slider::updateFillFromValue(float value) {
    float ratio = float(value - m_min) / float(m_max - m_min);
    ratio = std::max(0.f, std::min(ratio, 1.f));
    m_sliderLine->m_fillRatio = ratio;
    updateThumbPosition();
}

void Slider::updateThumbPosition() {
    const float fill_ratio = m_sliderLine->m_fillRatio.get();
    m_sliderPoint->m_vars.pos.relative.setAll({ fill_ratio, 0.5f });
    m_sliderPoint->m_vars.pos.absolute.setAll({ -(fill_ratio * 2.0f - 1.0f) * m_sliderPadding, 0.0f });
}

}