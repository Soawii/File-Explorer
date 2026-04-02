#pragma once
#include "EngineContext.hpp"
#include "../UI/UIManager.hpp"

class Engine {
public:
    EngineContext   m_context;
    ui::UIManager*  m_UIManager;

    Engine();

    void createWindowHandler(   sf::VideoMode mode, const sf::String &title, 
                                sf::Uint32 style = 7U, const sf::ContextSettings &settings = sf::ContextSettings());
    void createUIManager();
    
    void startFrame();
    void endFrame();
    void handleEvent(sf::Event& e);
    void update(float dt);
    void draw();
};