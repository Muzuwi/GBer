#include <string>
#pragma once
namespace Config{
	bool parseArgs(char*[], int);
	std::string getKeyState(std::string);
	void setKeyState(std::string, std::string);
	void setDefaults();
}  // namespace Config
