#pragma once
#include "UIElement.hpp"
#include <vector>

namespace ui {
    class Button : public UIElement {
    public:
        Button();

        void click();

        bool m_radio = false;
        std::vector<Button*>* m_radioGroup;
    };
}