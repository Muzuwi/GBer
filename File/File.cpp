#include "File.hpp"
namespace File{
		unsigned int fileSize = 0;

        bool fileExists(std::string path){
            boost::filesystem::path p(path);
            return (boost::filesystem::exists(p) & boost::filesystem::is_regular_file(p)) ? true : false;
        }

        std::vector<char> loadFromFile(std::string path){
        	std::ifstream file;
        	std::vector<char> rom;
        	file.open(path);
			file.seekg(0, file.end);
            fileSize = file.tellg();
            file.seekg(0, file.beg);
            
            rom.resize(fileSize);
            file.read(&rom[0], fileSize);
            file.close();

            return rom;
        }
}