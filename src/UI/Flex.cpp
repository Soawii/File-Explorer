#include "Flex.hpp"

ui::Flex::Flex() {} 

void ui::Flex::computeSize() {
    if (m_context->fitContentFixed && m_context->contentSizeComputed > 0) {
        if (m_context->dontComputeOutOfBoundsChildren) {
            computeBounds();
            for (int i = 0; i < m_children.size(); i++) {
                m_children[i]->propogateCall([](UIElement* e) {
                    const float p_left =    e->m_parent->m_context->childrenBounds.m_pos.x;
                    const float p_top =     e->m_parent->m_context->childrenBounds.m_pos.y;
                    const float p_bottom =  e->m_parent->m_context->childrenBounds.m_pos.y + e->m_parent->m_context->childrenBounds.m_height;
                    const float p_right =   e->m_parent->m_context->childrenBounds.m_pos.x + e->m_parent->m_context->childrenBounds.m_width;
                    
                    float left =    e->m_context->anchorBounds.m_pos.x;
                    float top =     e->m_context->anchorBounds.m_pos.y;
                    float bottom =  e->m_context->anchorBounds.m_pos.y + e->m_context->anchorBounds.m_height;
                    float right =   e->m_context->anchorBounds.m_pos.x + e->m_context->anchorBounds.m_width;

                    e->computeBounds();

                    if (!(bottom < p_top || right < p_left || left > p_right || top > p_bottom))
                        return true;
                    left =      e->m_context->anchorBounds.m_pos.x;
                    top =       e->m_context->anchorBounds.m_pos.y;
                    bottom =    e->m_context->anchorBounds.m_pos.y + e->m_context->anchorBounds.m_height;
                    right =     e->m_context->anchorBounds.m_pos.x + e->m_context->anchorBounds.m_width;
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

    for (UIElement* child : m_children) {
        child->computeSize();
    }

    std::vector<UIElement*> sortedChildren = m_children;
    std::stable_sort(sortedChildren.begin(), sortedChildren.end(), [](UIElement* a, UIElement* b) {
        return a->m_vars.flex_order.get() < b->m_vars.flex_order.get();
    });
    if (m_reverse) {
        std::reverse(sortedChildren.begin(), sortedChildren.end());
    }

    int childrenAmount = 0;
    float totalMain = 0.0f;
    float maxCross = 0.0f;

    for (UIElement* child : sortedChildren) {
        const sf::Vector2f cb = child->getLocalBounds();
        const float flex = child->m_vars.flex.get();

        if (child->m_context->m_current.state == UIElementState::DISABLED || child->m_context->dontCountTowardsLayout) {
            continue;
        }

        childrenAmount++;

        if (m_direction == FlexDirection::horizontal) {
            if (flex == 0.0f)
                totalMain += cb.x;
            maxCross = std::max(maxCross, cb.y);
        } else {
            if (flex == 0.0f)
                totalMain += cb.y;
            maxCross = std::max(maxCross, cb.x);
        }
    }
    const float totalGaps = (childrenAmount > 1 ? m_gap * (childrenAmount - 1) + m_startGap + m_endGap : 0.0f);
    bool recomputeChildren = false;

    if (m_context->sizeMode[0] == UISizeMode::FIT_CONTENT || 
        m_context->sizeMode[1] == UISizeMode::FIT_CONTENT || 
        m_context->overflow[0] == UIOverflowMode::SCROLL || 
        m_context->overflow[1] == UIOverflowMode::SCROLL) 
    {
        const float borderWidth = m_vars.borderWidth.get();
        const float padding = m_vars.padding.get();
        const float prev_scrollWidthX = m_context->scrollbarActualWidth[0];
        const float prev_scrollWidthY = m_context->scrollbarActualWidth[1];

        const sf::Vector2f size = m_vars.size.absolute.get();
        sf::Vector2f newSize = size;

        if (m_context->sizeMode[0] == UISizeMode::FIT_CONTENT) {
            newSize.x = (m_direction == FlexDirection::horizontal ? (totalMain + totalGaps) : maxCross)
                + borderWidth * 2 + padding * 2 + m_context->scrollbarActualWidth[1];
        }
        if (m_context->overflow[0] == UIOverflowMode::SCROLL) {
            m_context->scrollCalulated[0] = (m_direction == FlexDirection::horizontal ? (totalMain + totalGaps) : maxCross)
                + borderWidth + padding * 2;
            const float scrollbar_lengthX = m_context->scrollCalulated[0] * ((m_context->mouseBounds.m_width < 0.1f) ? 1.0f : (m_context->mouseBounds.m_width / std::max(m_context->mouseBounds.m_width, m_context->scrollCalulated[0])));
            const float scrollPosX = m_context->scrollPosX.get();
            const float maxScrollPosX = std::max(0.0f, m_context->scrollCalulated[0] - scrollbar_lengthX);
            if (scrollPosX > maxScrollPosX) {
                m_context->scrollPosX.setInstantly(maxScrollPosX);
                recomputeChildren = true;
            }
        }
        if (m_context->sizeMode[1] == UISizeMode::FIT_CONTENT) {
            newSize.y = (m_direction == FlexDirection::vertical ? (totalMain + totalGaps) : maxCross)
                + borderWidth * 2 + padding * 2 + m_context->scrollbarActualWidth[0];
        }
        if (m_context->overflow[1] == UIOverflowMode::SCROLL) {
            m_context->scrollCalulated[1] = (m_direction == FlexDirection::vertical ? (totalMain + totalGaps) : maxCross)
                + borderWidth + padding * 2;
            const float scrollbar_lengthY = m_context->scrollCalulated[1] * ((m_context->mouseBounds.m_height < 0.1f) ? 1.0f : (m_context->mouseBounds.m_height / std::max(m_context->mouseBounds.m_height, m_context->scrollCalulated[1])));
            const float scrollPosY = m_context->scrollPosY.get();
            const float maxScrollPosY = std::max(0.0f, m_context->scrollCalulated[1] - scrollbar_lengthY);
            if (scrollPosY > maxScrollPosY) {
                m_context->scrollPosY.setInstantly(maxScrollPosY);
                recomputeChildren = true;
            }
        }

        if (m_context->sizeMode[0] == UISizeMode::FIT_CONTENT || m_context->sizeMode[1] == UISizeMode::FIT_CONTENT) {
            if (fabs(size.x - newSize.x) > 0.1f || fabs(size.y - newSize.y) > 0.1f) {
                m_vars.size.absolute.setAllSmoothly(newSize);
                recomputeChildren = true;
            }
        }
        computeBounds();
        if (m_context->overflow[0] == UIOverflowMode::SCROLL || m_context->overflow[1] == UIOverflowMode::SCROLL) {
            if (fabs(prev_scrollWidthX - m_context->scrollbarActualWidth[0]) > 0.01f 
                || fabs(prev_scrollWidthY - m_context->scrollbarActualWidth[1]) > 0.01f) {
                recomputeChildren = true;
            }
        }
    }

    std::vector<sf::Vector2f> childBounds;
    std::vector<float> childFlex;
    for (UIElement* child : sortedChildren) {
        const sf::Vector2f cb = child->getLocalBounds();
        const float flex = child->m_vars.flex.get();
        childBounds.push_back(cb);
        childFlex.push_back(flex);
    }

    const Bounds& contentBounds = m_context->childrenBounds;

    const float containerMainSize = (m_direction == FlexDirection::horizontal) ? contentBounds.m_width : contentBounds.m_height;
    const float containerCrossSize = (m_direction == FlexDirection::horizontal) ? contentBounds.m_height : contentBounds.m_width;

    float remainingSpace = containerMainSize - totalMain;
    float startOffset = 0.0f;
    float spacing = m_gap;

    float totalFlex = 0.0f;
    for (int i = 0; i < sortedChildren.size(); i++) {
        totalFlex += childFlex[i];
    }
    if (totalFlex > 0.0f) {
        float availableMain = containerMainSize - (totalMain + totalGaps);
        if (availableMain < 0.0f)
            availableMain = 0.0f;
        for (int i = 0; i < sortedChildren.size(); i++) {
            UIElement* child = sortedChildren[i];
            if (child->m_context->m_current.state == UIElementState::DISABLED || child->m_context->dontCountTowardsLayout) {
                continue;
            }
            const float flexValue = childFlex[i];
            if (flexValue > 0.0f) {
                const sf::Vector2f childSize = child->m_vars.size.absolute.get();
                sf::Vector2f newChildSize = childSize;
                if (m_direction == FlexDirection::horizontal) {
                    newChildSize.x = (flexValue / totalFlex) * availableMain;
                } else {
                    newChildSize.y = (flexValue / totalFlex) * availableMain;
                }
                if (childSize != newChildSize) {
                    child->m_vars.size.absolute.setAll(newChildSize);
                    childBounds[i] = child->getLocalBounds();
                    recomputeChildren = true;
                }
            }
        }
        remainingSpace = totalGaps;
    }

    switch (m_justify) {
        case JustifyContent::start:
            startOffset = m_startGap;
            break;
        case JustifyContent::end:
            startOffset = remainingSpace - totalGaps;
            break;
        case JustifyContent::center:
            startOffset = (remainingSpace - totalGaps) / 2.0f;
            break;
        case JustifyContent::space_between:
            spacing = (childrenAmount > 1) ? (remainingSpace / (childrenAmount - 1)) : 0.0f;
            break;
        case JustifyContent::space_around:
            spacing = (childrenAmount > 0) ? (remainingSpace / childrenAmount) : 0.0f;
            startOffset = spacing / 2.0f;
            break;
    }

    float currentPos = startOffset;
    for (int i = 0; i < sortedChildren.size(); i++) {
        UIElement* child = sortedChildren[i];
        if (child->m_context->m_current.state == UIElementState::DISABLED || child->m_context->dontCountTowardsLayout) {
            continue;
        }
        const sf::Vector2f& cb = childBounds[i];

        const sf::Vector2f childPos = child->m_vars.pos.absolute.get();
        sf::Vector2f newChildPos = childPos;

        if (m_direction == FlexDirection::horizontal) {
            newChildPos.x = currentPos;
            switch (m_align) {
                case AlignItems::start:
                    newChildPos.y = 0.0f;
                    break;
                case AlignItems::end:
                    newChildPos.y = containerCrossSize - cb.y;
                    break;
                case AlignItems::center:
                    newChildPos.y = (containerCrossSize - cb.y) / 2.0f;
                    break;
            }
            currentPos += cb.x + spacing;
        } else {
            newChildPos.y = currentPos;
            switch (m_align) {
                case AlignItems::start:
                    newChildPos.x = 0.0f;
                    break;
                case AlignItems::end:
                    newChildPos.x = containerCrossSize - cb.x;
                    break;
                case AlignItems::center:
                    newChildPos.x = (containerCrossSize - cb.x) / 2.0f;
                    break;
            }
            currentPos += cb.y + spacing;
        }
        if (childPos != newChildPos) {
            child->m_vars.pos.absolute.setAllSmoothly(newChildPos);
            recomputeChildren = true;
        }
    }

    if (recomputeChildren) {
        for (UIElement* c : m_children) {
            if (c->m_context->contentSizeComputed > 0)
                c->m_context->contentSizeComputed = 0;
            c->computeSize();
            c->checkDirtyBounds();
        }
    }

    if (m_context->contentSizeComputed < 5)
        m_context->contentSizeComputed++;
}