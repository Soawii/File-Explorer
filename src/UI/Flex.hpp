#pragma once
#include "UIElement.hpp"

namespace ui {
    enum class FlexDirection {
        horizontal = 0,
        vertical = 1
    };

    enum class JustifyContent {
        start = 0,
        end = 1,
        space_between = 2,
        space_around = 3,
        center = 4
    };

    enum class AlignItems {
        start = 0,
        end = 1,
        center = 2
    };

    class Flex : public UIElement {
    public:
        FlexDirection m_direction = FlexDirection::horizontal;
        JustifyContent m_justify = JustifyContent::start;
        AlignItems m_align = AlignItems::start;
        float m_gap = 0.0f, m_startGap = 0.0f, m_endGap = 0.0f;
        bool m_reverse = false;

        Flex();

        void computeSize() override;
    };
}