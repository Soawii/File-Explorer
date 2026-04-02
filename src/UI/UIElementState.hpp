#pragma once

namespace ui {

enum class UIElementState {
    HIDDEN = 0,
    DISABLED = 1,
    NORMAL = 2,
    HOVER = 3,
    ACTIVE = 4,
    PRESSED = 5,
    FOCUSED = 6,
    STATE_AMOUNT
};

enum class UISizeMode {
    FIXED = 0,
    FIT_CONTENT = 1
};

enum class UIOverflowMode {
    VISIBLE = 0,
    SCROLL,
    HIDDEN
};

}