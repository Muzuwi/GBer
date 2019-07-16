#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <fstream>

namespace Utils{
    std::string decHex(int num, unsigned int size=4);
    bool exists(std::string path);
    std::vector<uint8_t> loadFromFile(std::string path);
}