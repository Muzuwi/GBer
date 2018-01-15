#pragma once
#include "boost/filesystem.hpp"
#include <vector>
#include <fstream>

namespace File{
	bool fileExists(std::string);
	std::vector<unsigned char> loadFromFile(std::string);
}
