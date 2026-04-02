#include "conf.hpp"
#include <iostream>

namespace conf {
	float constants::PI = 3.14159265359f;

	int window::WIDTH = 1200;
	int window::HEIGHT = 800;
	sf::Vector2u window::minSize = {600, 400};
	sf::Vector2u window::maxSize = {10000, 10000};

	int settings::FPS = 144;
	bool settings::smoothScroll = true;
	bool settings::smoothSort = true;
	float settings::scrollDist = 80.0f;

	sf::Color colors::backgroundColor = sf::Color(30, 30, 30);
	sf::Color colors::jointColor = sf::Color(255, 255, 255);
	sf::Color colors::jointConnectionColor = sf::Color(255, 255, 255);
	sf::Color colors::baseColor = sf::Color(57, 65, 100);
	sf::Color colors::weightColor = sf::Color(118, 86, 29);
	sf::Color colors::weightOutlineColor = sf::Color(255, 172, 28);
	sf::Color colors::sliderColor = sf::Color(0, 0, 0, 0);
	sf::Color colors::sliderOutlineColor = sf::Color(230, 230, 230);

	sf::Font fonts::mono_r;
	sf::Font fonts::mono_r_semibold;
	sf::Font fonts::emojis;
	sf::Font fonts::emoji_type;

	std::chrono::steady_clock::time_point time::now;

	void init() {
		fonts::mono_r.loadFromFile("../resources/fonts/mono-r.ttf");
		fonts::mono_r_semibold.loadFromFile("../resources/fonts/mono-r-semibold.ttf");
		fonts::emojis.loadFromFile("../resources/fonts/emojis.ttf");
		fonts::emoji_type.loadFromFile("../resources/fonts/emoji-type.ttf");

		std::ifstream settings_file("../resources/settings.txt", std::ios::in);
		if (!settings_file || 
			!(settings_file >> settings::FPS) ||
			!(settings_file >> settings::smoothScroll) ||
			!(settings_file >> settings::smoothSort) ||
			!(settings_file >> settings::scrollDist)) 
		{
			settings::FPS = 144;
			settings::smoothScroll = true;
			settings::smoothSort = true;
			settings::scrollDist = 80.0f;
		} 

		time::now = std::chrono::steady_clock::now();
	}

	void destroy() {
		std::ofstream settings_file("../resources/settings.txt", std::ios::out | std::ios::trunc);
		if (settings_file) {
			settings_file << settings::FPS << "\n" << settings::smoothScroll << "\n" << settings::smoothSort << "\n" << settings::scrollDist << "\n";
		}
	}
}