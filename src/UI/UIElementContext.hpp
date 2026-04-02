#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <vector>
#include <array>
#include <chrono>
#include <any>
#include "../util/TimingVariable.hpp"
#include "Color.hpp"
#include "UIElementState.hpp"
#include "Bounds.hpp"
#include "MouseContext.hpp"
#include "Shapes.hpp"

namespace ui {

class Keypress {
public:
    Keypress(sf::Keyboard::Key KEY, bool SHIFT_MATTERS, bool SHIFT, bool CTRL_MATTERS, bool CTRL) 
    : key(KEY), shift_matters(SHIFT_MATTERS), shift(SHIFT), ctrl_matters(CTRL_MATTERS), ctrl(CTRL) {}

    sf::Keyboard::Key key;
    bool shift_matters = false, shift = false;
    bool ctrl_matters = false, ctrl = false;
};

struct UIElementFrameContext {
    UIElementState state = UIElementState::NORMAL;
    bool isMouseIn = false;
};

class UIElementContext {
public:
    UIElementContext();

    UIElementFrameContext m_prev, m_current;

    Bounds childrenBounds, anchorBounds, mouseBounds;
    bool dirtyBounds = false, dirtyTransform = false;

    int z_index = 0;

    int contentSizeComputed = 0, transformComputed = 0;
    int minSizePasses = 0, minTransformPasses = 0;
 
    // optimization flags
    bool fitContentFixed = false, transformFixed = false;
    bool dontComputeOutOfBoundsChildren = false; // used with fitContentFixed and usually scroll elements
    bool statelessElement = false, statelessChildren = false;
    bool copyTransformToChildren = false;
    bool dontDrawChildren = false;
    bool dontUpdateChildren = false, dontUpdateYourself = false;
    bool dontComputeSizeOfChildren = false, dontComputeSizeOfYourself = false;
    bool checkBoundsOfChildren = false; // should only be enabled with dontComputeSizeOfChildren

    bool dontCountTowardsLayout = false;
    bool ignoreParentsTranform = false;

    // states
    bool pushed = false;                        // active state, when we pressed left click on it but have not released it
    bool pressable = false, pressed = false;
    bool focusable = false, focused = false;

    sf::Transform calculatedTransform;
    float calculatedOpacity;

    UISizeMode sizeMode[2] = {UISizeMode::FIXED, UISizeMode::FIXED};
    bool includeBorderPadding = false;

    // scroll
    UIOverflowMode overflow[2] = {UIOverflowMode::VISIBLE, UIOverflowMode::VISIBLE};
    float scrollbarActualWidth[2] = {0.0f, 0.0f};
    float scrollbarWidth[2] = {0.0f, 0.0f};
    float scrollbarBorderRadius = 10.0f;
    float scrollWheelDistance = 80.0f;
    TimingVariable<float> scrollPosX, scrollPosY;
    float scrollCalulated[2] = {0.0f, 0.0f};
    bool mousePressedOnScroll[2] = {false, false};
    float mousePressedOnScrollPos[2] = {0.0f, 0.0f};
    bool autoScroll = false;

    // evnet handling
    bool mousePressedHere = false;
    bool propogateMouse[sf::Mouse::ButtonCount];
    bool propogateKeyboard[sf::Keyboard::KeyCount];
    bool propogateHover = false, propogateScroll = true;
    std::vector<std::function<void(MouseContext&)>> onClick;
    std::vector<std::function<void(MouseContext&)>> onPress[2];
    std::vector<std::function<void(MouseContext&)>> onMouseEnter;
    std::vector<std::function<void(MouseContext&)>> onMouseLeave;
    std::vector<std::function<void()>> onScrollChange;
    std::vector<std::function<void()>> whileActive;
    std::vector<std::function<void()>> onUnfocus;
    std::vector<std::function<void()>> onStateChange[size_t(UIElementState::STATE_AMOUNT)];

    std::vector<std::pair<Keypress, std::function<void()>>> onPressKeys;
    std::vector<std::pair<Keypress, std::function<void()>>> onTriggerKeys;
    std::unordered_map<sf::Uint32, std::function<void()>> onTextEnter;

    std::vector<std::pair<std::chrono::steady_clock::time_point, std::function<void(void)>>> callbacksOnTimepoint;

    std::vector<std::function<void(sf::RenderWindow*)>> endOfDraw;

    std::string id;
    
    size_t data_id = 0;
    size_t data_type = 0;
    std::any data_any;

    bool didMouseMoveIn();
    bool didMouseMoveOut();

    bool wasStateChanged();
    void triggerEvents(const std::vector<std::function<void()>>& events);
    void triggerEvents(const std::vector<std::function<void(MouseContext&)>>& events, MouseContext& context);
    void triggerEvents(const std::vector<std::function<void(MouseContext&)>>& events, MouseContext&& context);
    void changedState();

    void startFrame();
    void endFrame();
};

}