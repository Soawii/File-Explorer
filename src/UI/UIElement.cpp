#include "UIElement.hpp"
#include <SFML/OpenGL.hpp>
#include "ShapePool.hpp"
#include "DrawBatchGroup.hpp"

namespace ui {

UIElement::UIElement() : m_vars(m_context) {
}

UIElement::~UIElement() {
    for (int i = 0; i < m_children.size(); i++) {
        delete m_children[i];
    }
    delete m_context;
}

sf::Vector2f UIElement::getLocalBounds() {
    const Bounds& sizeRect = m_context->includeBorderPadding ? m_parent->m_context->anchorBounds : m_parent->m_context->childrenBounds;
    const sf::Vector2f absoluteSize = m_vars.size.absolute.get();
    const sf::Vector2f relativeSize = m_vars.size.relative.get();
    return sf::Vector2f(
        relativeSize.x * sizeRect.m_width + absoluteSize.x,
        relativeSize.y * sizeRect.m_height + absoluteSize.y
    );
}

sf::FloatRect UIElement::getGlobalBounds() {
    const Bounds& posRect = ((m_parent == m_anchor && !m_context->includeBorderPadding) ? m_anchor->m_context->childrenBounds : m_anchor->m_context->anchorBounds);

    const sf::Vector2f absoluteSize = m_vars.size.absolute.get();
    const sf::Vector2f relativeSize = m_vars.size.relative.get();
    const sf::Vector2f localBounds(getLocalBounds());

    const sf::Vector2f absolutePos = m_vars.pos.absolute.get();
    const sf::Vector2f relativePos = m_vars.pos.relative.get();
    const sf::Vector2f absoluteOrigin = m_vars.origin.absolute.get();
    const sf::Vector2f relativeOrigin = m_vars.origin.relative.get();

    const sf::Vector2f pos = {
        relativePos.x * posRect.m_width 
        + absolutePos.x + posRect.m_pos.x
        - absoluteOrigin.x - relativeOrigin.x * localBounds.x
        - m_parent->m_context->scrollPosX.get(),

        relativePos.y * posRect.m_height 
        + absolutePos.y + posRect.m_pos.y
        - absoluteOrigin.y - relativeOrigin.y * localBounds.y
        - m_parent->m_context->scrollPosY.get()
    };

    return sf::FloatRect(pos.x, pos.y, localBounds.x, localBounds.y);
}

void UIElement::computeBounds() {
    const float borderWidth = m_vars.borderWidth.get();
    const float padding = m_vars.padding.get();
    const sf::FloatRect bounds = getGlobalBounds();

    m_context->scrollbarActualWidth[0] = 0.0f;
    m_context->scrollbarActualWidth[1] = 0.0f;
    if (m_context->overflow[0] == UIOverflowMode::SCROLL && (!m_context->autoScroll || (m_context->scrollCalulated[0] > (bounds.width - borderWidth))))
        m_context->scrollbarActualWidth[0] = m_context->scrollbarWidth[0];
    if (m_context->overflow[1] == UIOverflowMode::SCROLL && (!m_context->autoScroll || (m_context->scrollCalulated[1] > (bounds.height - borderWidth))))
        m_context->scrollbarActualWidth[1] = m_context->scrollbarWidth[1];

    m_context->mouseBounds = Bounds(
        {bounds.left, bounds.top}, 
        bounds.width - (m_context->scrollbarActualWidth[1] > 0.5f ? borderWidth + m_context->scrollbarActualWidth[1] : 0.0f), 
        bounds.height - (m_context->scrollbarActualWidth[0] > 0.5f ? borderWidth + m_context->scrollbarActualWidth[0] : 0.0f));
    m_context->mouseBounds.m_radius = std::max(0.0f, m_vars.borderRadius.get() - borderWidth);

    m_context->childrenBounds = Bounds(
        { bounds.left + borderWidth + padding,
        bounds.top + borderWidth + padding}, 
        bounds.width - borderWidth * 2 - padding * 2 - m_context->scrollbarActualWidth[1],
        bounds.height - borderWidth * 2 - padding * 2 - m_context->scrollbarActualWidth[0]
    );

    m_context->anchorBounds = Bounds(
        {bounds.left, bounds.top},
        bounds.width, bounds.height);
    m_context->anchorBounds.m_radius = m_context->mouseBounds.m_radius;
}

void UIElement::computeSize() {
    if (m_context->fitContentFixed && m_context->contentSizeComputed > 0) {
        // If an element has the m_context->dontComputeOutOfBoundsChildren flag
        // set to true - don't update the children of invisible elements.
        // Huge optimization when there are a lot of children inside a scrollable element
        // that have their own children that don't have to get updates until they are visible.
        // (Like file name or size texts inside the file button)
        if (m_context->dontComputeOutOfBoundsChildren) {
            computeBounds();
            for (int i = 0; i < m_children.size(); i++) {
                m_children[i]->propogateCall([](UIElement* e) {
                    const float p_left = e->m_parent->m_context->childrenBounds.m_pos.x;
                    const float p_top = e->m_parent->m_context->childrenBounds.m_pos.y;
                    const float p_bottom = e->m_parent->m_context->childrenBounds.m_pos.y + e->m_parent->m_context->childrenBounds.m_height;
                    const float p_right = e->m_parent->m_context->childrenBounds.m_pos.x + e->m_parent->m_context->childrenBounds.m_width;
                    
                    float left = e->m_context->anchorBounds.m_pos.x;
                    float top = e->m_context->anchorBounds.m_pos.y;
                    float bottom = e->m_context->anchorBounds.m_pos.y + e->m_context->anchorBounds.m_height;
                    float right = e->m_context->anchorBounds.m_pos.x + e->m_context->anchorBounds.m_width;

                    e->computeBounds();

                    if (!(bottom < p_top || right < p_left || left > p_right || top > p_bottom))
                        return true;
                    left = e->m_context->anchorBounds.m_pos.x;
                    top = e->m_context->anchorBounds.m_pos.y;
                    bottom = e->m_context->anchorBounds.m_pos.y + e->m_context->anchorBounds.m_height;
                    right = e->m_context->anchorBounds.m_pos.x + e->m_context->anchorBounds.m_width;
                    return !(bottom < p_top || right < p_left || left > p_right || top > p_bottom);
                });
            }
        }
        else {
            propogateCall([](UIElement* e) {
                e->computeBounds();
                return true;
            });
        }
        return;
    }

    computeBounds();

    for (int i = 0; i < m_children.size(); i++) {
        m_children[i]->computeSize();
    }

    // If an element is FIT_CONTENT or overflow: SCROLL, compute the bounding box
    // of all children and update the Element Size or the Scroll Size accordingly.
    if (    m_context->sizeMode[0] == UISizeMode::FIT_CONTENT || 
            m_context->sizeMode[1] == UISizeMode::FIT_CONTENT || 
            m_context->overflow[0] == UIOverflowMode::SCROLL || 
            m_context->overflow[1] == UIOverflowMode::SCROLL
        ) {
        const float borderWidth = m_vars.borderWidth.get();
        const float padding = m_vars.padding.get();
        const float prev_scrollWidthX = m_context->scrollbarActualWidth[0];
        const float prev_scrollWidthY = m_context->scrollbarActualWidth[1];
        bool recalculateChildren = false;
        const sf::Vector2f size = m_vars.size.absolute.m_var.m_var.getActual();
        sf::Vector2f newSize = size;
        
        if (m_context->sizeMode[0] == UISizeMode::FIT_CONTENT || m_context->overflow[0] == UIOverflowMode::SCROLL) {
            if (m_children.size() == 0) {
                newSize.x = borderWidth * 2 + padding * 2 + m_context->scrollbarActualWidth[1];
            }
            else {
                float x_min = 10000000.0f;
                float x_max = -10000000.0f;

                for (int i = 0; i < m_children.size(); i++) {
                    UIElement* c = m_children[i];
                    if (c->m_context->m_current.state == UIElementState::DISABLED || c->m_context->dontCountTowardsLayout)
                        continue;

                    sf::FloatRect bounds = c->getGlobalBounds();
                    x_min = std::min(x_min, bounds.left);
                    x_max = std::max(x_max, bounds.left + bounds.width);
                }
                newSize.x = (x_max - x_min) + borderWidth * 2 + padding * 2 + m_context->scrollbarActualWidth[1];
            }
            if (m_context->overflow[0] == UIOverflowMode::SCROLL) {
                m_context->scrollCalulated[0] = newSize.x - borderWidth - m_context->scrollbarActualWidth[1];
                const float scrollbar_lengthX = m_context->scrollCalulated[0] * ((m_context->mouseBounds.m_width < 0.1f) ? 1.0f : (m_context->mouseBounds.m_width / std::max(m_context->mouseBounds.m_width, m_context->scrollCalulated[0])));
                const float scrollPosX = m_context->scrollPosX.get();
                const float maxScrollPosX = std::max(0.0f, m_context->scrollCalulated[0] - scrollbar_lengthX);
                if (scrollPosX > maxScrollPosX) {
                    m_context->scrollPosX.setInstantly(maxScrollPosX);
                    recalculateChildren = true;
                }
            }
        }
        if (m_context->sizeMode[1] == UISizeMode::FIT_CONTENT || m_context->overflow[1] == UIOverflowMode::SCROLL) {
            if (m_children.size() == 0) {
                newSize.y = borderWidth * 2 + padding * 2 + m_context->scrollbarActualWidth[0];
            }
            else {
                float y_min = 10000000.0f;
                float y_max = -10000000.0f;

                for (int i = 0; i < m_children.size(); i++) {
                    UIElement* c = m_children[i];
                    if (c->m_context->m_current.state == UIElementState::DISABLED || c->m_context->dontCountTowardsLayout)
                        continue;

                    sf::FloatRect bounds = c->getGlobalBounds();
                    y_min = std::min(y_min, bounds.top);
                    y_max = std::max(y_max, bounds.top + bounds.height);
                }
                newSize.y = (y_max - y_min) + borderWidth * 2 + padding * 2 + m_context->scrollbarActualWidth[0];
            }
            if (m_context->overflow[1] == UIOverflowMode::SCROLL) {
                m_context->scrollCalulated[1] = newSize.y - borderWidth - m_context->scrollbarActualWidth[0];
                const float scrollbar_lengthY = m_context->scrollCalulated[1] * ((m_context->mouseBounds.m_height < 0.1f) ? 1.0f : (m_context->mouseBounds.m_height / std::max(m_context->mouseBounds.m_height, m_context->scrollCalulated[1])));
                const float scrollPosY = m_context->scrollPosY.get();
                const float maxScrollPosY = std::max(0.0f, m_context->scrollCalulated[1] - scrollbar_lengthY);
                if (scrollPosY > maxScrollPosY) {
                    m_context->scrollPosY.setInstantly(maxScrollPosY);
                    recalculateChildren = true;
                }
            }
        }

        if ((m_context->sizeMode[0] == UISizeMode::FIT_CONTENT || m_context->sizeMode[1] == UISizeMode::FIT_CONTENT) 
                && (fabs(newSize.x - size.x) > 0.001 || fabs(newSize.y - size.y) > 0.001)) {
            m_vars.size.absolute.setAllSmoothly(newSize);
            recalculateChildren = true;
        }
        computeBounds();

        // Children might need recalculation if we changed the size or the scroll position.
        if (recalculateChildren
            || fabs(prev_scrollWidthX - m_context->scrollbarActualWidth[0]) > 0.01f 
            || fabs(prev_scrollWidthY - m_context->scrollbarActualWidth[1]) > 0.01f) {
            for (int i = 0; i < m_children.size(); i++) {
                m_children[i]->computeSize();
                m_children[i]->checkDirtyBounds();
            }
        }
    }
    if (m_context->contentSizeComputed < 5)
        m_context->contentSizeComputed++;
}

void UIElement::computeTransforms() {
    // Compute the transform and opacity of the element and its children.
    // It's combined with the calculated transform and opacity of the parent [parentTransform, parentOpacity].

    // This is done after the computeSize() function because transformOrigin and translate.relative depend on the size of an element.
    // For example, if we set relative translate X to -0.5f, the element will move 50% of its size to the left.
    // This is done so we can easily center elements without using element's origin or flex elements.

    if (m_context->transformFixed && m_context->transformComputed > 0) {
        return;
    }

    const Bounds& bounds = m_context->anchorBounds;

    const float scale = m_vars.scale.get();
    const float rotate = m_vars.rotate.get();
    const sf::Vector2f absTranslate = m_vars.translate.absolute.get();
    const sf::Vector2f relTranslate = m_vars.translate.relative.get();
    const sf::Vector2f absTransformOrigin = m_vars.transformOrigin.absolute.get();
    const sf::Vector2f relTransformOrigin = m_vars.transformOrigin.relative.get();

    const sf::Vector2f translate = {
        absTranslate.x + relTranslate.x * bounds.m_width,
        absTranslate.y + relTranslate.y * bounds.m_height
    };
    const sf::Vector2f transformOrigin = {
        bounds.m_pos.x + absTransformOrigin.x + relTransformOrigin.x * bounds.m_width,
        bounds.m_pos.y + absTransformOrigin.y + relTransformOrigin.y * bounds.m_height
    };

    sf::Transform t;
    t.scale(sf::Vector2f(scale, scale), transformOrigin);
    t.rotate(rotate, transformOrigin);
    t.translate(translate);
    const float opacity = m_vars.opacity.get();

    const sf::Transform totalTransform = ((m_parent && !m_context->ignoreParentsTranform) ? m_parent->m_context->calculatedTransform : sf::Transform()) * t;
    const float totalOpacity = ((m_parent && !m_context->ignoreParentsTranform) ? m_parent->m_context->calculatedOpacity : 1.0f) * opacity;

    if (m_context->copyTransformToChildren) {
        propogateCall([totalTransform, totalOpacity](UIElement* e) {
            e->m_context->calculatedTransform = totalTransform;
            e->m_context->mouseBounds.m_transform = totalTransform;
            e->m_context->anchorBounds.m_transform = totalTransform;
            e->m_context->childrenBounds.m_transform = totalTransform;
            e->m_context->calculatedOpacity = totalOpacity;
            return true;
        });
    }
    else {
        m_context->calculatedTransform = totalTransform;
        m_context->mouseBounds.m_transform = totalTransform;
        m_context->childrenBounds.m_transform = totalTransform;
        m_context->anchorBounds.m_transform = totalTransform;
        m_context->calculatedOpacity = totalOpacity;
        for (UIElement* child : m_children) {
            child->computeTransforms();
        }
    }
    if (m_context->transformComputed < 5)
        m_context->transformComputed++;
}

void UIElement::propogateCall(const std::function<bool(UIElement*)>& func, bool toVisibleChildren) {
    const bool continue_propogation = func(this);
    if (continue_propogation) {
        if (toVisibleChildren && (
                m_context->overflow[0] != UIOverflowMode::VISIBLE 
                || m_context->overflow[1] != UIOverflowMode::VISIBLE)) {

            const float top = m_context->anchorBounds.m_pos.y;
            const float left = m_context->anchorBounds.m_pos.x;
            const float bottom = m_context->anchorBounds.m_pos.y + m_context->anchorBounds.m_height;
            const float right = m_context->anchorBounds.m_pos.x + m_context->anchorBounds.m_width;

            for (int i = 0; i < m_children.size(); i++) {

                const float c_top = m_children[i]->m_context->anchorBounds.m_pos.y;
                const float c_left = m_children[i]->m_context->anchorBounds.m_pos.x;
                const float c_bottom = m_children[i]->m_context->anchorBounds.m_pos.y + m_children[i]->m_context->anchorBounds.m_height;
                const float c_right = m_children[i]->m_context->anchorBounds.m_pos.x + m_children[i]->m_context->anchorBounds.m_width;

                if (c_bottom < top || 
                    c_right < left ||
                    c_left > right || 
                    c_top > bottom) {
                        continue;
                    }
                
                m_children[i]->propogateCall(func, toVisibleChildren);
            }
        }
        else {
            for (int i = 0; i < m_children.size(); i++) {
                m_children[i]->propogateCall(func, toVisibleChildren);
            }
        }
    }
}

void UIElement::draw(sf::RenderWindow* window, sf::IntRect& viewport) {
    // Return if an element is outside the viewport
    const float top = m_context->anchorBounds.m_pos.y;
    const float left = m_context->anchorBounds.m_pos.x;
    const float bottom = m_context->anchorBounds.m_pos.y + m_context->anchorBounds.m_height;
    const float right = m_context->anchorBounds.m_pos.x + m_context->anchorBounds.m_width;
    if (bottom < viewport.top || 
        right < viewport.left ||
        left > viewport.left + viewport.width || 
        top > viewport.top + viewport.height) 
    {
        return;
    }

    // Get all the values of state variables, draw the main rounded outline rectangle shape.
    const float borderWidth = m_vars.borderWidth.get();
    const float borderRadius = m_vars.borderRadius.get();
    const Color colorHSL = m_vars.color.get();
    const Color borderColorHSL = m_vars.borderColor.get();
    sf::Color color = hslToRgb(colorHSL);
    sf::Color borderColor = hslToRgb(borderColorHSL);
    color.a *= m_context->calculatedOpacity;
    borderColor.a *= m_context->calculatedOpacity;

    sf::Transform calculatedTransform = m_context->calculatedTransform;
    calculatedTransform.translate(m_context->anchorBounds.m_pos);

    const float shadowBlurLength = m_vars.shadowBlurLength.get();
    const sf::Vector2f shadowOffset = m_vars.shadowOffset.get();
    if (fabs(shadowBlurLength) > 0.5f || fabs(shadowOffset.x) > 0.5f || fabs(shadowOffset.y) > 0.5f) 
    {
        const Color shadowStartHSL = m_vars.shadowColorStart.get();
        const Color shadowEndHSL = m_vars.shadowColorEnd.get();
        sf::Color shadowStart = hslToRgb(shadowStartHSL);
        sf::Color shadowEnd = hslToRgb(shadowEndHSL);
        shadowStart.a *= m_context->calculatedOpacity;
        shadowEnd.a *= m_context->calculatedOpacity;
        
        if (shadowStart.a > 0.5f || shadowEnd.a > 0.5f) 
        {
            const RoundedOutlinedRectParams shadow_p(
                m_context->anchorBounds.m_width + 2 * shadowBlurLength, 
                m_context->anchorBounds.m_height + 2 * shadowBlurLength,
                shadowBlurLength, borderRadius + shadowBlurLength, 10);
            const shapes::RoundedOutlinedRect *shadow_shape = 
                ShapePool::findOrCreateRoundedOutlinedRect(shadow_p, shadowStart, shadowStart, shadowEnd);

            sf::Transform shadowTransform = calculatedTransform;
            shadowTransform.translate(shadowOffset - sf::Vector2f(shadowBlurLength, shadowBlurLength));

            if (fabs(shadowBlurLength) < 0.5f) {
                window->draw(shadow_shape->m_inner, shadowTransform);
            }
            else if (fabs(shadowOffset.x) < 0.5f && fabs(shadowOffset.y) < 0.5f) {
                window->draw(shadow_shape->m_outer, shadowTransform);
            }
            else {
                shadow_shape->draw(*window, shadowTransform);
            }
        }
    }

    const RoundedOutlinedRectParams p(m_context->anchorBounds.m_width, m_context->anchorBounds.m_height, borderWidth, borderRadius, 5);
    const shapes::RoundedOutlinedRect *shape = ShapePool::findOrCreateRoundedOutlinedRect(p, color, borderColor);
    shape->draw(*window, calculatedTransform);

    // If overflow is hidden or scroll, apply a mask with bounds equal to m_context->anchorBounds so the children don't overflow the parent.
    // Also, update the viewport so we don't waste draw calls on invisible elements.
    if (m_context->overflow[0] == UIOverflowMode::HIDDEN ||
        m_context->overflow[1] == UIOverflowMode::HIDDEN ||
        m_context->overflow[0] == UIOverflowMode::SCROLL || 
        m_context->overflow[1] == UIOverflowMode::SCROLL) 
    {
        const sf::IntRect oldView = viewport;
        viewport.left = m_context->anchorBounds.m_pos.x + borderWidth;
        viewport.top = m_context->anchorBounds.m_pos.y + borderWidth;
        viewport.width = m_context->anchorBounds.m_width - borderWidth * 2;
        viewport.height = m_context->anchorBounds.m_height - borderWidth * 2;

        static int stencilDepth = 0;
        stencilDepth++;
        if (stencilDepth == 1) {
            glEnable(GL_STENCIL_TEST);
        }
        // Increase the bits of the parent mask with 'GL_INCR' so an element with 'overflow: hidden'
        // inside another 'overflow: hidden' doesn't overflow the calculated parent's mask
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glStencilFunc(GL_EQUAL, stencilDepth - 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

        const RoundedRectParams p_mask(m_context->anchorBounds.m_width - borderWidth * 2, m_context->anchorBounds.m_height - borderWidth * 2, std::max(0.0f, borderRadius - borderWidth), 5);
        shapes::RoundedRect *mask_shape = ShapePool::findOrCreateRoundedRect(p_mask, sf::Color(255,255,255));
        calculatedTransform.translate({ borderWidth , borderWidth });
        mask_shape->draw(*window, calculatedTransform);

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glStencilFunc(GL_EQUAL, stencilDepth, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        // Get all the children to draw.
        // If a child is also 'overflow: hidden', we don't want to draw it inside this function
        //  - instead we call its own draw function so it applies its own mask as well.

        std::vector<UIElement*> drawables;

        propogateCall([this, &drawables](UIElement* e) {
            if (e->m_context->m_current.state == UIElementState::HIDDEN || e->m_context->calculatedOpacity < 0.001f) {
                return false;
            }
            bool temp = (e == this);
            if (!temp)
                drawables.push_back(e);
            return temp || 
                !(e->m_context->overflow[0] == UIOverflowMode::HIDDEN ||
                e->m_context->overflow[1] == UIOverflowMode::HIDDEN ||
                e->m_context->overflow[0] == UIOverflowMode::SCROLL || 
                e->m_context->overflow[1] == UIOverflowMode::SCROLL || 
                e->m_context->dontDrawChildren);
        }, true);

        std::stable_sort(drawables.begin(), drawables.end(), [](UIElement* l, UIElement* r) {
            return l->m_context->z_index < r->m_context->z_index;
        });

        // Update the viewport to current element's bounds and draw the children.
        window->pushGLStates();
        for (UIElement* c : drawables) {
            c->draw(window, viewport);
        }
        for (int i = 0; i < m_context->endOfDraw.size(); i++) {
            m_context->endOfDraw[i](window);
        }
        window->popGLStates();
        viewport = oldView;


        // If and element is 'overflow: scroll' in any direction, draw the scrollbar.
        if (m_context->overflow[0] == UIOverflowMode::SCROLL || m_context->overflow[1] == UIOverflowMode::SCROLL) {
            const Color scrollHSL = m_vars.scrollBackgroundColor.get();
            const Color scrollbarHSL = m_vars.scrollColor.get();
            sf::Color scroll = hslToRgb(scrollHSL);
            sf::Color scrollbar = hslToRgb(scrollbarHSL);
            scroll.a *= m_context->calculatedOpacity;
            scrollbar.a *= m_context->calculatedOpacity;

            window->pushGLStates();
            if (m_context->overflow[0] == UIOverflowMode::SCROLL && m_context->scrollbarActualWidth[0] > 0.5f) {
                const RoundedRectParams x_scroll(m_context->mouseBounds.m_width - borderWidth, m_context->scrollbarActualWidth[0], 0.0f, 5);
                const RoundedOutlinedRectParams x_scrollbar(
                    (m_context->mouseBounds.m_width - borderWidth) * (m_context->mouseBounds.m_width / std::max(m_context->mouseBounds.m_width, m_context->scrollCalulated[0])),
                    m_context->scrollbarActualWidth[0], 0.0f, m_context->scrollbarBorderRadius, 5);

                const shapes::RoundedRect *scroll_shape = ShapePool::findOrCreateRoundedRect(x_scroll, scroll);
                const shapes::RoundedOutlinedRect *scrollbar_shape = ShapePool::findOrCreateRoundedOutlinedRect(x_scrollbar, scrollbar, sf::Color::Transparent);
                
                sf::Transform t = calculatedTransform;
                t.translate({0.0f, m_context->mouseBounds.m_height - borderWidth});
                scroll_shape->draw(*window, t);
                t.translate({m_context->mouseBounds.m_width * (m_context->scrollPosX.get() / std::max(1.0f, m_context->scrollCalulated[0])), 0.0f});
                scrollbar_shape->draw(*window, t);
            }
            if (m_context->overflow[1] == UIOverflowMode::SCROLL && m_context->scrollbarActualWidth[1] > 0.5f) {
                const RoundedRectParams y_scroll(m_context->scrollbarActualWidth[1], m_context->mouseBounds.m_height, 1.0f, 5);
                const RoundedOutlinedRectParams y_scrollbar(
                    m_context->scrollbarActualWidth[1],
                    (m_context->mouseBounds.m_height - borderWidth) * (m_context->mouseBounds.m_height / std::max(m_context->mouseBounds.m_height, m_context->scrollCalulated[1])),
                    0.0f, m_context->scrollbarBorderRadius, 5);

                const shapes::RoundedRect *scroll_shape = ShapePool::findOrCreateRoundedRect(y_scroll, scroll);
                const shapes::RoundedOutlinedRect *scrollbar_shape = ShapePool::findOrCreateRoundedOutlinedRect(y_scrollbar, scrollbar, sf::Color::Transparent);
                
                sf::Transform t = calculatedTransform;
                t.translate({m_context->mouseBounds.m_width - borderWidth, 0.0f});
                scroll_shape->draw(*window, t);
                t.translate({0.0f, m_context->mouseBounds.m_height * (m_context->scrollPosY.get() / std::max(1.0f, m_context->scrollCalulated[1]))});
                scrollbar_shape->draw(*window, t);
            }
            window->popGLStates();
        }

        // After drawing all the children, we want to remove the mask so it doesnt't the future draws.
        // We do it by reversing eariler 'GL_INCR' by drawing the calculated mask with 'GL_DECR' instead.
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glStencilFunc(GL_EQUAL, stencilDepth, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);

        window->pushGLStates();
        mask_shape->draw(*window, calculatedTransform);
        window->popGLStates();

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        
        stencilDepth--;
        if (stencilDepth == 0) {
            glDisable(GL_STENCIL_TEST);
        } else {
            // If the element is not the first drawn 'oveflow: hidden' element in the tree
            // set the mask equal to the mask of the previously drawn (usually parent) 'overflow: hidden' element.  
            glStencilFunc(GL_EQUAL, stencilDepth, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        }
    }
}

void UIElement::update(MouseContext& mouseContext, KeyboardContext& keyboardContext) {
    if (m_context->m_current.state == UIElementState::HIDDEN || m_context->m_current.state == UIElementState::DISABLED) {
        m_context->mousePressedHere = false;
        return;
    }

    const bool isMouseInMouseBounds = m_context->mouseBounds.doesPointIntersect(sf::Vector2f(mouseContext.m_current.pos));
    bool isMouseIn = !mouseContext.m_current.hoverConsumed && isMouseInMouseBounds;
    if (isMouseIn && m_parent != nullptr && 
            (m_parent->m_context->overflow[0] == UIOverflowMode::HIDDEN || m_parent->m_context->overflow[1] == UIOverflowMode::HIDDEN ||
            m_parent->m_context->overflow[0] == UIOverflowMode::SCROLL || m_parent->m_context->overflow[1] == UIOverflowMode::SCROLL)) {
        isMouseIn = isMouseIn && m_parent->m_context->mouseBounds.doesPointIntersect(sf::Vector2f(mouseContext.m_current.pos));
    }
    m_context->m_current.isMouseIn = isMouseIn;
    if (isMouseIn && !m_context->propogateHover) {
        mouseContext.consumeHover();
    }
    if (isMouseInMouseBounds && !m_context->propogateScroll) {
        mouseContext.consumeScroll();
    }

    if (isMouseIn && !m_context->m_prev.isMouseIn) {
        m_context->triggerEvents(m_context->onMouseEnter, mouseContext);
    }
    if (!isMouseIn && m_context->m_prev.isMouseIn) {
        m_context->triggerEvents(m_context->onMouseLeave, mouseContext);
    }

    for (int i = 0; i < 2; i++) {
        if (isMouseIn && mouseContext.isPressedThisFrame((sf::Mouse::Button)i) && !mouseContext.isConsumedThisFrame((sf::Mouse::Button)i)) {
            if (i == 0)
                m_context->mousePressedHere = true;
            m_context->triggerEvents(m_context->onPress[i], mouseContext);
            if (!m_context->propogateMouse[(sf::Mouse::Button)i]) {
                mouseContext.consume((sf::Mouse::Button)i);
            }
        }
    }
    if (mouseContext.isReleasedThisFrame(sf::Mouse::Left)) {
        if (m_context->mousePressedHere) {
            if (isMouseIn) {
                m_context->triggerEvents(m_context->onClick, mouseContext);
            }
            if (!m_context->propogateMouse[sf::Mouse::Left])
                mouseContext.m_current.isConsumed[sf::Mouse::Left] = false;
        }
        m_context->mousePressedHere = false;
    }

    if (m_context->focusable && m_context->focused && !isMouseInMouseBounds && mouseContext.isPressedThisFrame(sf::Mouse::Left)) {
        m_context->triggerEvents(m_context->onUnfocus);
        m_context->focused = false;
    }

    if (m_context->focusable && m_context->focused) {
        for (sf::Uint32 c : keyboardContext.m_current.textEnteredThisFrame) {
            if (m_context->onTextEnter.find(c) != m_context->onTextEnter.end())
                m_context->onTextEnter[c]();
        }
        for (int i = 0; i < m_context->onPressKeys.size(); i++) {
            const Keypress keypress = m_context->onPressKeys[i].first;
            if (keyboardContext.isPressedThisFrame(keypress.key) && !keyboardContext.isConsumedThisFrame(keypress.key)
                && (!keypress.shift_matters || keypress.shift == keyboardContext.isPressed(sf::Keyboard::LShift))
                && (!keypress.ctrl_matters || keypress.ctrl == keyboardContext.isPressed(sf::Keyboard::LControl))) 
            {
                m_context->onPressKeys[i].second();
                if (!m_context->propogateKeyboard[keypress.key])
                    keyboardContext.consume(keypress.key);
            }
        }
        for (int i = 0; i < m_context->onPressKeys.size(); i++) {
            const Keypress keypress = m_context->onPressKeys[i].first;
            if (keyboardContext.isReleasedThisFrame(keypress.key) && keyboardContext.isConsumed(keypress.key)) {
                keyboardContext.m_current.isConsumed[keypress.key] = false;
                keyboardContext.m_current.isConsumedThisFrame[keypress.key] = false;
            }
        }
        for (int i = 0; i < m_context->onTriggerKeys.size(); i++) {
            const Keypress keypress = m_context->onTriggerKeys[i].first;
            if (keyboardContext.isTriggeredThisFrame(keypress.key) && !keyboardContext.isConsumedThisFrame(keypress.key)
                && (!keypress.shift_matters || keypress.shift == keyboardContext.isPressed(sf::Keyboard::LShift))
                && (!keypress.ctrl_matters || keypress.ctrl == keyboardContext.isPressed(sf::Keyboard::LControl))) 
            {
                m_context->onTriggerKeys[i].second();
                if (!m_context->propogateKeyboard[keypress.key])
                    keyboardContext.consume(keypress.key);
            }
        }
        for (int i = 0; i < m_context->onTriggerKeys.size(); i++) {
            const Keypress keypress = m_context->onTriggerKeys[i].first;
            if (keyboardContext.isReleasedThisFrame(keypress.key) && keyboardContext.isConsumed(keypress.key)) {
                keyboardContext.m_current.isConsumed[keypress.key] = false;
                keyboardContext.m_current.isConsumedThisFrame[keypress.key] = false;
            }
        }
    }

    // updating scroll
    if (m_context->overflow[0] == UIOverflowMode::SCROLL || m_context->overflow[1] == UIOverflowMode::SCROLL) {
        const bool isMouseInMouseBounds = m_context->mouseBounds.doesPointIntersect(sf::Vector2f(mouseContext.m_current.pos));
        bool isMouseInScroll = !isMouseInMouseBounds && !mouseContext.m_current.hoverConsumed && m_context->anchorBounds.doesPointIntersect(sf::Vector2f(mouseContext.m_current.pos));
        const sf::Vector2f point_inversed = m_context->anchorBounds.m_transform.getInverse().transformPoint(sf::Vector2f(mouseContext.m_current.pos));
        if (isMouseInScroll) {
            if (point_inversed.x - m_context->mouseBounds.m_pos.x > m_context->mouseBounds.m_width + m_context->scrollbarActualWidth[1]) {
                isMouseInScroll = false;
            }
            else if (point_inversed.y - m_context->mouseBounds.m_pos.y > m_context->mouseBounds.m_height + m_context->scrollbarActualWidth[0]) {
                isMouseInScroll = false;
            }
        }

        UIElementState state = UIElementState::NORMAL;

        if (isMouseInScroll)
            state = UIElementState::HOVER;

        if (isMouseInScroll && mouseContext.isPressedThisFrame(sf::Mouse::Left) && !mouseContext.isConsumedThisFrame(sf::Mouse::Left)) {
            if (m_context->overflow[0] == UIOverflowMode::SCROLL && 
                point_inversed.y - m_context->anchorBounds.m_pos.y > m_context->mouseBounds.m_height) 
            {
                m_context->mousePressedOnScroll[0] = true;
                const float scrollbar_width = m_context->mouseBounds.m_width / std::max(m_context->mouseBounds.m_width, m_context->scrollCalulated[0]);
                const float scrollbar_pos_x = m_context->scrollPosX.get() / m_context->scrollCalulated[0];
                const float point_inversed_x = (point_inversed.x - m_context->anchorBounds.m_pos.x) / m_context->mouseBounds.m_width;
                if (point_inversed_x >= scrollbar_pos_x && point_inversed_x <= scrollbar_pos_x + scrollbar_width) {
                    m_context->mousePressedOnScrollPos[0] = (point_inversed_x - scrollbar_pos_x) / scrollbar_width;
                }
                else {
                    m_context->mousePressedOnScrollPos[0] = 0.5f;
                }
                m_context->mousePressedOnScroll[0] = true;
                mouseContext.consume(sf::Mouse::Left);
            }
            if (m_context->overflow[1] == UIOverflowMode::SCROLL && 
                point_inversed.x - m_context->anchorBounds.m_pos.x > m_context->mouseBounds.m_width) 
            {
                m_context->mousePressedOnScroll[1] = true;
                const float scrollbar_height = m_context->mouseBounds.m_height / std::max(m_context->mouseBounds.m_height, m_context->scrollCalulated[1]);
                const float scrollbar_pos_y = m_context->scrollPosY.get() / m_context->scrollCalulated[1];
                const float point_inversed_y = (point_inversed.y - m_context->anchorBounds.m_pos.y) / m_context->mouseBounds.m_height;
                if (point_inversed_y >= scrollbar_pos_y && point_inversed_y <= scrollbar_pos_y + scrollbar_height) {
                    m_context->mousePressedOnScrollPos[1] = (point_inversed_y - scrollbar_pos_y) / scrollbar_height;
                }
                else {
                    m_context->mousePressedOnScrollPos[1] = 0.5f;
                }
                mouseContext.consume(sf::Mouse::Left);
            }
        }
        if (mouseContext.isReleasedThisFrame(sf::Mouse::Left)) {
            if (m_context->mousePressedOnScroll[0] || m_context->mousePressedOnScroll[1]) {
                mouseContext.m_current.isConsumed[sf::Mouse::Left] = false;
            }
            m_context->mousePressedOnScroll[0] = false;
            m_context->mousePressedOnScroll[1] = false;
        }

        if (m_context->mousePressedOnScroll[0]) {
            state = UIElementState::ACTIVE;
            const float scrollbar_width = m_context->mouseBounds.m_width * (m_context->mouseBounds.m_width / std::max(m_context->mouseBounds.m_width, m_context->scrollCalulated[0]));
            const float ratio = std::max(0.0f, std::min(1.0f, (point_inversed.x - m_context->anchorBounds.m_pos.x - m_context->mousePressedOnScrollPos[0] * scrollbar_width) / (m_context->anchorBounds.m_width - scrollbar_width)));
            m_context->scrollPosX.setInstantly(ratio * (m_context->scrollCalulated[0] * (1.0f - scrollbar_width / m_context->mouseBounds.m_width)));
        }
        else if (m_context->mousePressedOnScroll[1]) {
            state = UIElementState::ACTIVE;
            const float scrollbar_height = m_context->mouseBounds.m_height * (m_context->mouseBounds.m_height / std::max(m_context->mouseBounds.m_height, m_context->scrollCalulated[1]));
            const float ratio = std::max(0.0f, std::min(1.0f, (point_inversed.y - m_context->anchorBounds.m_pos.y - m_context->mousePressedOnScrollPos[1] * scrollbar_height) / (m_context->anchorBounds.m_height - scrollbar_height)));
            m_context->scrollPosY.setInstantly(ratio * (m_context->scrollCalulated[1] * (1.0f - scrollbar_height / m_context->mouseBounds.m_height)));
            
        }

        if (isMouseInMouseBounds && mouseContext.m_current.mouseWheelScrolled) {
            mouseContext.m_current.mouseWheelScrolled = false;
            if (m_context->overflow[1] == UIOverflowMode::SCROLL) {
                const float scrollbar_height = m_context->mouseBounds.m_height * (m_context->mouseBounds.m_height / std::max(m_context->mouseBounds.m_height, m_context->scrollCalulated[1]));
                const float initial_pos = m_context->scrollPosY.getActual();
                const float new_pos = std::max(
                    0.0f, 
                    std::min(
                        initial_pos - m_context->scrollWheelDistance * mouseContext.m_current.scrollDelta,
                        m_context->scrollCalulated[1] * (1.0f - scrollbar_height / m_context->mouseBounds.m_height)));
                
                if (fabs(initial_pos - new_pos) > 0.5f) {
                    m_context->scrollPosY = new_pos;
                    m_context->triggerEvents(m_context->onScrollChange);
                }
            }
            else {
                const float scrollbar_width = m_context->mouseBounds.m_width * (m_context->mouseBounds.m_width / std::max(m_context->mouseBounds.m_width, m_context->scrollCalulated[0]));
                const float initial_pos = m_context->scrollPosX.getActual();
                const float new_pos = std::max(
                    0.0f, 
                    std::min(
                        initial_pos - m_context->scrollWheelDistance * mouseContext.m_current.scrollDelta,
                        m_context->scrollCalulated[0] * (1.0f - scrollbar_width / m_context->mouseBounds.m_width)));
                
                if (fabs(initial_pos - new_pos) > 0.5f) {
                    m_context->scrollPosX = new_pos;
                    m_context->triggerEvents(m_context->onScrollChange);
                }
            }
        }

        m_vars.scrollColor.stateChanged(state);
        m_vars.scrollBackgroundColor.stateChanged(state);
    }

    if (m_context->m_current.state == UIElementState::HIDDEN || m_context->m_current.state == UIElementState::DISABLED) {
        m_context->mousePressedHere = false;
        return;
    }

    if (m_context->mousePressedHere || m_context->pushed) {
        m_context->m_current.state = UIElementState::ACTIVE;
    } else if (isMouseIn) {
        m_context->m_current.state = UIElementState::HOVER;
    } else {
        m_context->m_current.state = UIElementState::NORMAL;
    }
    if (m_context->pressable && m_context->pressed) {
        m_context->m_current.state = UIElementState::PRESSED;
    }
    if (m_context->focusable && m_context->focused) {
        m_context->m_current.state = UIElementState::FOCUSED;
    }

    if (m_context->m_current.state == UIElementState::ACTIVE) {
        m_context->triggerEvents(m_context->whileActive);
    }

    if (m_context->callbacksOnTimepoint.size() > 0) {
        auto it = std::partition(m_context->callbacksOnTimepoint.begin(), m_context->callbacksOnTimepoint.end(), [](const auto& a) {
            return a.first > conf::time::now;
        });
        if (it != m_context->callbacksOnTimepoint.end()) {
            for (; it != m_context->callbacksOnTimepoint.end(); it++) {
                it->second();
            }
            m_context->callbacksOnTimepoint.resize(it - m_context->callbacksOnTimepoint.begin());
        }
    }

    if (m_context->wasStateChanged()) {
        m_context->triggerEvents(m_context->onStateChange[size_t(m_context->m_current.state)]);
    }

    for (IUIAnimation* anim : m_animations) {
        anim->update();
    }
}

bool UIElement::checkDirtyBounds() {
    // Using | instead || because we need all wasValueChanged() functions to be called
    // so the dirty flags get consumed and elements don't get computed next frames for no reason

    bool isDirtyPropogate = false;
    bool isDirtyNoPropogate = false;

    if (!m_context->dontComputeSizeOfYourself) {
        isDirtyPropogate =      m_vars.size.absolute.m_var.m_var.wasValueChanged() | 
                                m_vars.size.relative.m_var.m_var.wasValueChanged() |
                                m_vars.pos.absolute.m_var.m_var.wasValueChanged() |
                                m_vars.pos.relative.m_var.m_var.wasValueChanged() | 
                                m_vars.origin.absolute.m_var.m_var.wasValueChanged() | 
                                m_vars.origin.relative.m_var.m_var.wasValueChanged() | 
                                m_vars.flex.m_var.m_var.wasValueChanged() | 
                                m_vars.flex_order.m_var.m_var.wasValueChanged();

        isDirtyNoPropogate =    m_context->scrollPosX.wasValueChanged() |
                                m_context->scrollPosY.wasValueChanged() ||
                                (m_context->m_prev.state == UIElementState::DISABLED && m_context->m_current.state != UIElementState::DISABLED) ||
                                (m_context->m_prev.state != UIElementState::DISABLED && m_context->m_current.state == UIElementState::DISABLED) ||
                                m_anchor->m_context->dirtyBounds ||
                                m_context->dirtyBounds;
    }


    bool childDirty = false;
    if (!m_context->dontComputeSizeOfChildren) {
        for (UIElement* child : m_children) {
            childDirty = childDirty | child->checkDirtyBounds();
        }
    }

    if (isDirtyPropogate) {
        m_context->dirtyBounds = true;
        return true;
    }
    if (isDirtyNoPropogate) {
        m_context->dirtyBounds = true;
        return false;
    }
    
    if (m_context->m_current.state == UIElementState::DISABLED) {
        return false;
    }

    if (childDirty && (
            m_context->sizeMode[0] == UISizeMode::FIT_CONTENT || 
            m_context->sizeMode[1] == UISizeMode::FIT_CONTENT ||
            m_context->overflow[0] == UIOverflowMode::SCROLL || 
            m_context->overflow[1] == UIOverflowMode::SCROLL) &&
            (!m_context->fitContentFixed || m_context->contentSizeComputed == 0)) {
        m_context->dirtyBounds = true;
        return true;
    }
    return false;
}
void UIElement::checkDirtyTransforms() {
    bool isDirty = false;
    if (!m_context->dontComputeSizeOfYourself) {
        isDirty =   m_vars.translate.absolute.m_var.m_var.wasValueChanged() | 
                    m_vars.translate.relative.m_var.m_var.wasValueChanged() |
                    m_vars.rotate.m_var.m_var.wasValueChanged() |
                    m_vars.scale.m_var.m_var.wasValueChanged() | 
                    m_vars.opacity.m_var.m_var.wasValueChanged();
    };

    if (!m_context->dontComputeSizeOfChildren) {
        for (UIElement* child : m_children) {
            child->checkDirtyTransforms();
        }
    }
    
    if (isDirty) {
        m_context->dirtyTransform = true;
        return;
    }
    if (m_context->m_current.state == UIElementState::DISABLED) {
        return;
    }
}

void UIElement::setState(UIElementState state) {
    m_context->m_prev.state = m_context->m_current.state;
    m_context->m_current.state = state;
    checkChangedStates();
}

void UIElement::checkChangedStates() {
    m_vars.checkChangedStates();
}

void UIElement::addChild(UIElement* child, UIElement* anchor) {
    child->m_parent = this;
    child->m_anchor = anchor ? anchor : this;
    child->m_context->z_index = m_context->z_index;
    m_children.push_back(child);
}

UIElement* UIElement::getChildById(std::string id) {
    if (m_context->id == id) {
        return this;
    }
    for (UIElement* c : m_children) {
        UIElement* found = c->getChildById(id);
        if (found != nullptr) {
            return found;
        } 
    }
    return nullptr;
}

void UIElement::setScrollPos(float pos, bool y_axis, bool smooth) {
    if (!y_axis) {
        const float scrollbar_length = m_context->scrollCalulated[0] * (m_context->mouseBounds.m_width / std::max(m_context->mouseBounds.m_width, m_context->scrollCalulated[0]));
        const float maxScrollPos = std::max(0.0f, m_context->scrollCalulated[0] - scrollbar_length);
        pos = std::max(0.0f, std::min(pos, maxScrollPos));
        if (smooth)
            m_context->scrollPosX = pos;
        else
            m_context->scrollPosX.setInstantly(pos);
    }
    else {
        const float scrollbar_length = m_context->scrollCalulated[1] * (m_context->mouseBounds.m_height / std::max(m_context->mouseBounds.m_height, m_context->scrollCalulated[1]));
        const float maxScrollPos = std::max(0.0f, m_context->scrollCalulated[1] - scrollbar_length);
        pos = std::max(0.0f, std::min(pos, maxScrollPos));
        if (smooth)
            m_context->scrollPosY = pos;
        else
            m_context->scrollPosY.setInstantly(pos);
    }
}

}