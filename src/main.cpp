#include <SFML/Graphics.hpp>
#include <iostream>
#include <iomanip>
#include <thread>
#include "conf.hpp"
#include "engine/Engine.hpp"
#include "util/util.hpp"
#include "engine/sync.hpp"
#include "filesystem/FileExplorer.hpp"

int main() {
    conf::init();
    sync::init();
    FileSystem::init();

    Engine engine;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    settings.stencilBits = 8;
    engine.createWindowHandler(sf::VideoMode(conf::window::WIDTH, conf::window::HEIGHT), "File Explorer", sf::Style::Default, settings);
    engine.m_context.m_window->setFramerateLimit(conf::settings::FPS);
    engine.createUIManager();

    FileExplorer file_explorer(engine.m_UIManager); 

    while (engine.m_context.m_window->isOpen()) {
        sf::Event event;

        engine.startFrame();

        while (engine.m_context.m_window->pollEvent(event)) {
            engine.handleEvent(event);
        }

        engine.update(1.0f / conf::settings::FPS);

        engine.draw();

        engine.endFrame();
        
        engine.m_context.m_window->display();
    }
    conf::destroy();

    return 0;
}