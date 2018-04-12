#include <iostream>
#include <fstream>
#include <string>
#include "Debug.hpp"
#include "../Math/Math.hpp"
#include <SFML/Graphics.hpp>
#include "../Memory/RAM.hpp"
#include <boost/lexical_cast.hpp>

namespace Debug{
	sf::RenderWindow mainWindow;
	sf::Font font;
 	sf::Text text;
 	int offset = 0;
 	/*bool logToFile = true;
 	std::ofstream logFile;*/
	/*
		Log a given message to the console/file
	*/

	void emuLog(std::string message, LEVEL lvl){
		//  Add fancy colors some other time
		/*if(logFile){
			if(!logFile.is_open()){
				logFile.open("EMULOG.txt");
			}
			logFile << 
		}*/
		std::cout << (lvl == 10 ? "[@] " : \
					   lvl == 5 ? "[!] " : "[i] " ) << message << "\n";
 	}

 	/*
		Log a given message to the console/file
		If no level given, use INFO
 	*/
 	void emuLog(std::string message){
 		std::cout << "[i] " << message << "\n";
 	} 


 	void debugWindow(){
 		font.loadFromFile("CALIBRI.TTF");
  		text.setFont(font);

 		mainWindow.create(sf::VideoMode(1280,720), "Test1");
 		mainWindow.setFramerateLimit(60);
 		while(mainWindow.isOpen()){
 			sf::Event event;
 			while(mainWindow.pollEvent(event)){
 				switch(event.type){
 					case sf::Event::Closed: 
 						mainWindow.close();
 						break;
 					default:
 						break;
 				}
 			}
 			//  Magic numbers yay
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down)     && offset < 2016) offset += 1;
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)    && offset < 1984) offset += 32;
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up)       && offset != 0) offset -= 1;
        	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)     && offset > 0x20 - 1) offset -= 32;

 			mainWindow.clear(sf::Color(0xE0,0xE0,0xE0));
 			drawRAM();

 			mainWindow.display();
 		}
 	}

 	void drawRAM(){
 		//  Draw the background
 		sf::RectangleShape bg;
 		bg.setFillColor(sf::Color(0xF5,0xF5,0xF5));
 		bg.setPosition(5,5);
 		bg.setSize(sf::Vector2f(695,510));
 		mainWindow.draw(bg);


 		const int rowCount = 32, columnCount = 32;
 		//  Draw the addresses
 		for(int i = 0; i < rowCount; i++){
 			printText(sf::Vector2f(6,2 + i*16), "$" + Math::decHex(0 + (offset + i)*rowCount), sf::Color::Black);
 		}

 		//  Draw the hex
 		for(int i = 0; i < rowCount; i++){
 			for(int j = 0; j < columnCount; j++){
 				printText(sf::Vector2f(64 + (j*20), 2 + (i*16)), Math::decHex(RAM::RAM[(i + offset)*columnCount + j]) , sf::Color::Black);
 			}
 		}
 	}

 	void printText(sf::Vector2f pos, std::string label, sf::Color color){
   		text.setCharacterSize(15);
  		text.setPosition(pos);
  		text.setFillColor(color);
  		text.setString(label);
  		mainWindow.draw(text);
	}

}