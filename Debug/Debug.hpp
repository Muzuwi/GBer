#pragma once
#include <SFML/Graphics.hpp>
namespace Debug{
	extern sf::RenderWindow mainWindow;
	enum LEVEL{
		ERROR=10,
		WARN=5,
		INFO=1
	};
	void emuLog(std::string, LEVEL);
	void emuLog(std::string);
	void debugWindow();
	void drawRAM();
 	void printText(sf::Vector2f, std::string , sf::Color);
}