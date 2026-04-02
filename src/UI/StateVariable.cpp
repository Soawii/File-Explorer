#include "StateVariable.hpp"
#include <iostream>

namespace ui {

ContextStateVariableRelAbs::ContextStateVariableRelAbs(
        UIElementContext* context,
        sf::Vector2f absVar, 
        float absDuration, 
        float(*absTimingFunction)(float),
        sf::Vector2f relVar, 
        float relDuration, 
        float(*relTimingFunction)(float)
    ) 
    : absolute(context, absVar, absDuration, absTimingFunction)
    , relative(context, relVar, relDuration, relTimingFunction) {
}

void ContextStateVariableRelAbs::checkChangedState() {
    absolute.checkChangedState();
    relative.checkChangedState();
}

StateVariables::StateVariables(UIElementContext* defaultContext)
:   pos(defaultContext, 
        {0.0f, 0.0f}, 0.0f, TimingFunctions::easeInOutQuad,
        {0.0f, 0.0f}, 0.0f, TimingFunctions::easeInOutQuad),     
    size(defaultContext, 
        {0.0f, 0.0f}, 0.0f, TimingFunctions::easeInOutQuad,
        {0.0f, 0.0f}, 0.0f, TimingFunctions::easeInOutQuad),  
    origin(defaultContext, 
        {0.0f, 0.0f}, 0.0f, TimingFunctions::easeInOutQuad,
        {0.0f, 0.0f}, 0.0f, TimingFunctions::easeInOutQuad),  
    transformOrigin(defaultContext, 
        {0.0f, 0.0f}, 0.0f, TimingFunctions::easeInOutQuad,
        {0.5f, 0.5f}, 0.0f, TimingFunctions::easeInOutQuad),  

    translate(defaultContext, 
        {0.0f, 0.0f}, 0.0f, TimingFunctions::easeInOutQuad,
        {0.0f, 0.0f}, 0.0f, TimingFunctions::easeInOutQuad),  
    rotate(defaultContext, 0.0f, 0.0f, TimingFunctions::easeInOutQuad),
    scale(defaultContext, 1.0f, 0.0f, TimingFunctions::easeInOutQuad),
    opacity(defaultContext, 1.0f, 0.0f, TimingFunctions::easeInOutQuad),

    borderRadius(defaultContext, 0.0f, 0.0f, TimingFunctions::easeInOutQuad),
    padding(defaultContext, 0.0f, 0.0f, TimingFunctions::easeInOutQuad),
    borderWidth(defaultContext, 0.0f, 0.0f, TimingFunctions::easeInOutQuad),

    shadowOffset(defaultContext, {0.0f, 0.0f}, 0.0f, TimingFunctions::easeInOutQuad),
    shadowBlurLength(defaultContext, 0.0f, 0.0f, TimingFunctions::easeInOutQuad),

    color(defaultContext, Color(255, 255, 255), 0.0f, TimingFunctions::easeInOutQuad),
    borderColor(defaultContext, Color(255, 255, 255), 0.0f, TimingFunctions::easeInOutQuad),
    shadowColorStart(defaultContext, Color(0,0,0,0), 0.0f, TimingFunctions::easeInOutQuad),
    shadowColorEnd(defaultContext, Color(0,0,0,0), 0.0f, TimingFunctions::easeInOutQuad),
    scrollColor(Color(210, 210, 210), 0.0f, TimingFunctions::smoothstep),
    scrollBackgroundColor(Color(150, 150, 150), 0.0f, TimingFunctions::smoothstep),

    flex(defaultContext, 0.0f, 0.0f, TimingFunctions::easeInOutQuad),
    flex_order(defaultContext, 0, 0.0f, TimingFunctions::easeInOutQuad) {
}

void StateVariables::checkChangedStates() {
    pos.checkChangedState();
    size.checkChangedState();
    origin.checkChangedState();
    transformOrigin.checkChangedState();
    opacity.checkChangedState();

    translate.checkChangedState();
    rotate.checkChangedState();
    scale.checkChangedState();

    borderRadius.checkChangedState();
    borderWidth.checkChangedState();
    padding.checkChangedState();
    shadowBlurLength.checkChangedState();
    shadowOffset.checkChangedState();

    color.checkChangedState();
    borderColor.checkChangedState();
    shadowColorStart.checkChangedState();
    shadowColorEnd.checkChangedState();
    flex.checkChangedState();
    flex_order.checkChangedState();
}

}