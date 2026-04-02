#include "ProgressBar.hpp"
#include "ShapePool.hpp"

namespace ui {

ProgressBar::ProgressBar(std::function<float(void)>* getFillRatio) 
: m_fillColor(m_context, Color(255, 255, 255), 0.0f, TimingFunctions::easeInOutQuad),
m_getFillRatio(getFillRatio),
m_fillRatio(0.0f, 0.0f, TimingFunctions::smoothstep)
{
    m_vars.color.setAll(Color(255,255,255,0));
}

void ProgressBar::draw(sf::RenderWindow* window, sf::IntRect& viewport) {
    const float top = m_context->mouseBounds.m_pos.y;
    const float left = m_context->mouseBounds.m_pos.x;
    const float bottom = m_context->mouseBounds.m_pos.y + m_context->anchorBounds.m_height;
    const float right = m_context->mouseBounds.m_pos.x + m_context->anchorBounds.m_width;
    if (bottom < viewport.top || 
            right < viewport.left ||
            left > viewport.left + viewport.width || 
            top > viewport.top + viewport.height) {
        return;
    }

    const Bounds& bounds = m_context->mouseBounds;
    const float borderWidth = m_vars.borderWidth.get();
    const float borderRadius = m_vars.borderRadius.get();
    const float padding = m_vars.padding.get();
    const Color colorHSL = m_vars.color.get();
    const Color borderColorHSL = m_vars.borderColor.get();
    const Color fillColorHSL = m_fillColor.get();
    sf::Color color = hslToRgb(colorHSL);
    sf::Color borderColor = hslToRgb(borderColorHSL);
    sf::Color fillColor = hslToRgb(fillColorHSL);
    color.a *= m_context->calculatedOpacity;
    borderColor.a *= m_context->calculatedOpacity;
    fillColor.a *= m_context->calculatedOpacity;

    const RoundedOutlinedRectParams p(bounds.m_width, bounds.m_height, borderWidth, borderRadius, 5);
    const shapes::RoundedOutlinedRect *shape = ShapePool::findOrCreateRoundedOutlinedRect(p, color, borderColor);

    if (m_getFillRatio) {
        m_fillRatio = (*m_getFillRatio)();
    }
    const RoundedOutlinedRectParams p_fill((bounds.m_width - 2.0f * borderWidth - 2.0f * padding) * std::max(0.0f, std::min(m_fillRatio.get(), 1.0f)), bounds.m_height - 2.0f * borderWidth - 2.0f * padding, 0.0f, borderRadius - borderWidth - padding, 5);
    const shapes::RoundedOutlinedRect *shape_fill = ShapePool::findOrCreateRoundedOutlinedRect(p_fill, fillColor, sf::Color(0,0,0,0));
    
    sf::Transform calculatedTransform = m_context->calculatedTransform;
    calculatedTransform.translate(m_context->mouseBounds.m_pos);
    shape->draw(*window, calculatedTransform);
    calculatedTransform.translate({borderWidth + padding, borderWidth + padding});
    shape_fill->draw(*window, calculatedTransform);
}

void ProgressBar::checkChangedStates() {
    UIElement::checkChangedStates();
    m_fillColor.checkChangedState();
}

}