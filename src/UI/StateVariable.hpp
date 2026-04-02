#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include "../util/TimingVariable.hpp"
#include <string>
#include <vector>
#include "Color.hpp"
#include "UIElementContext.hpp"

namespace ui {

template <typename T>
class StateVariable {
public:
    StateVariable(T startingVar, float duration, float(*timingFunction)(float)) 
    : m_var(startingVar, duration, timingFunction) {
        setAll(startingVar);
    }

    void setDuration(float duration) {
        m_var.setDuration(duration);
    }

    void setTimingFunction(float(*timingFunction)(float)) {
        m_var.setTimingFunction(timingFunction);
    }

    void setAll(T var) {
        m_var.setInstantly(var);
        for (int i = 0; i < size_t(UIElementState::STATE_AMOUNT); i++) {
            m_onStateChange[i] = var;
        }
    }

    void setAllSmoothly(T var) {
        m_var = var;
        for (int i = 0; i < size_t(UIElementState::STATE_AMOUNT); i++) {
            m_onStateChange[i] = var;
        }
    }

    void setState(UIElementState state, T var) {
        m_onStateChange[size_t(state)] = var;
    }

    void stateChanged(UIElementState newState) {
        this->m_var = m_onStateChange[size_t(newState)];
    }

    T get() {
        return m_var.get();
    }

    TimingVariable<T> m_var;
    T m_onStateChange[size_t(UIElementState::STATE_AMOUNT)];
};

template<typename T>
class ContextStateVariable {
public:
    ContextStateVariable(
        UIElementContext* context, T startingVar,
        float duration, float(*timingFunction)(float))
    : m_context(context), m_startcontext(context), m_var(startingVar, duration, timingFunction) {}

    void setContext(UIElementContext* context) {
        m_context = context;
    }

    bool checkChangedState() {
        if (m_context->wasStateChanged()) {
            stateChanged(m_context->m_current.state);
            return true;
        }
        return false;
    }

    void setDuration(float duration) {
        m_var.setDuration(duration);
    }

    void setTimingFunction(float(*timingFunction)(float)) {
        m_var.setTimingFunction(timingFunction);
    }

    void setAll(T var) {
        m_var.setAll(var);
    }

    void setAllSmoothly(T var) {
        m_var.setAllSmoothly(var);
    }

    void setState(UIElementState state, T var) {
        m_var.setState(state, var);
    }

    void stateChanged(UIElementState newState) {
        m_var.stateChanged(newState);
    }

    T get() {
        return m_var.get();
    }

    StateVariable<T> m_var;
    UIElementContext* m_context;
    UIElementContext* m_startcontext;
};

class ContextStateVariableRelAbs {
public:
    ContextStateVariableRelAbs(
        UIElementContext* context,
        sf::Vector2f absVar, float absDuration, float(*absTimingFunction)(float),
        sf::Vector2f relVar, float relDuration, float(*relTimingFunction)(float));

    void checkChangedState();

    ContextStateVariable<sf::Vector2f> absolute;
    ContextStateVariable<sf::Vector2f> relative;
};

class StateVariables {
public:
    StateVariables(UIElementContext* defaultContext);

    void checkChangedStates();

    ContextStateVariableRelAbs pos;
    ContextStateVariableRelAbs size;
    ContextStateVariableRelAbs origin;
    ContextStateVariableRelAbs transformOrigin;

    ContextStateVariableRelAbs translate;

    ContextStateVariable<Color> color;
    ContextStateVariable<Color> borderColor;
    ContextStateVariable<Color> shadowColorStart;
    ContextStateVariable<Color> shadowColorEnd;
    StateVariable<Color> scrollColor;
    StateVariable<Color> scrollBackgroundColor;

    ContextStateVariable<float> rotate;
    ContextStateVariable<float> scale;
    ContextStateVariable<float> opacity;

    ContextStateVariable<float> borderRadius;
    ContextStateVariable<float> padding;
    ContextStateVariable<float> borderWidth;

    ContextStateVariable<float> shadowBlurLength;
    ContextStateVariable<sf::Vector2f> shadowOffset;

    ContextStateVariable<float> flex;
    ContextStateVariable<int> flex_order;
};

}