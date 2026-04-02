#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <chrono>
#include <functional>
#include <filesystem>
#include <fstream>

namespace conf {
    class constants {
    public:
        static float PI;
    };

    class window {
    public:
        static int WIDTH;
        static int HEIGHT;
        static sf::Vector2u minSize;
        static sf::Vector2u maxSize;
    };

    class settings {
    public:
        static int FPS;
        static bool smoothScroll;
        static bool smoothSort;
        static float scrollDist;
    };

    class colors {
    public:
        static sf::Color backgroundColor,
            baseColor,
            jointColor,
            jointConnectionColor,
            weightColor,
            weightOutlineColor,
            sliderColor,
            sliderOutlineColor;
    };

    class fonts {
    public:
        static sf::Font mono_r, mono_r_semibold;
        static sf::Font emojis, emoji_type;
    };

    class time {
    public:
        static std::chrono::steady_clock::time_point now;
    };

    void init();
    void destroy();
}