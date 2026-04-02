#include "Engine.hpp"
#include "sync.hpp"
#include "../util/util.hpp"
#include "../filesystem/FileSystem.hpp"
#include "WindowSizeLimits.hpp"
#include <chrono>

Engine::Engine() : m_context() {}

void Engine::createWindowHandler(   sf::VideoMode mode, const sf::String &title,
                                    sf::Uint32 style, const sf::ContextSettings &settings) 
{
    m_context.m_window = new sf::RenderWindow(mode, title, style, settings);
    setWindowSizeLimits(
        *m_context.m_window, 
        conf::window::minSize.x, conf::window::minSize.y,
        conf::window::maxSize.x, conf::window::maxSize.y);
}

void Engine::createUIManager() 
{
    m_UIManager = new ui::UIManager(m_context);
}

void Engine::startFrame() 
{
    conf::time::now = std::chrono::steady_clock::now();

    m_UIManager->startFrame();

    sync::StateManager::triggerCallbacks();
    sync::TaskManager::consumeTasks(conf::time::now);
}

void Engine::endFrame() 
{
    m_UIManager->endFrame();
}

void Engine::handleEvent(sf::Event& e) 
{
    if (e.type == sf::Event::Closed) {
        m_context.m_window->close();
        
    }
    else if (e.type == sf::Event::Resized) {
        const sf::View view(sf::FloatRect(0.0f, 0.0f, e.size.width, e.size.height));
        m_context.m_window->setView(view);
        conf::window::WIDTH = e.size.width;
        conf::window::HEIGHT = e.size.height;
    }
    m_UIManager->handleEvent(e);
}

void Engine::update(float dt) 
{
    m_UIManager->update();
}

void Engine::draw() 
{
    m_context.m_window->clear(conf::colors::backgroundColor);
    m_UIManager->draw();
}