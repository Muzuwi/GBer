#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <map>

std::string decHex(int num){   
	std::map<int, char> Hex;
	Hex[0]  = '0'; 
	Hex[1]  = '1'; 
	Hex[2]  = '2'; 
	Hex[3]  = '3'; 
	Hex[4]  = '4'; 
	Hex[5]  = '5'; 
	Hex[6]  = '6'; 
	Hex[7]  = '7'; 
	Hex[8]  = '8'; 
	Hex[9]  = '9'; 
	Hex[10] = 'A';
	Hex[11] = 'B';
	Hex[12] = 'C';
	Hex[13] = 'D';
	Hex[14] = 'E';
	Hex[15] = 'F';
	if(num == 0){
	    return "0";
	}
	std::string text = "";
	int b = num, current = 0, a = 0;
	do{
	    a = b % 16;
	    text += Hex[a];
	    b /= 16;
	    current++;
	}while(b != 0);
	/*while(text.size() % 2 != 0){
	   text.append(1, '0');
	} */
	std::reverse(text.begin(), text.end());
	
	return (text.size() <= 0) ? "?" : text; 
}


int main(){
	std::ofstream output;
	output.open("TEMPLATE2.txt");
/*
	for(int i = 0; i <= 15; i++){
		for(int j = 0; j <= 15; j++){
			output << "case 0x" << decHex(i) + decHex(j) << ": OPCodes::gb" << decHex(i) + decHex(j) << "(); break;\n";
		}
	}
	output << "\n\n\n\n\n\n\n";

	for(int i = 0; i <= 15; i++){
		for(int j = 0; j <= 15; j++){
			output << "case 0x" << decHex(i) + decHex(j) << ": OPCodesCB::cb" << decHex(i) + decHex(j) << "(); break;\n";
		}
	}

	output << "\n\n\n\n\n\n\n";*/

	for(int i = 0; i <= 15; i++){
		for(int j = 0; j <= 15; j++){
			output << "    void gb" << decHex(i) + decHex(j) << "(){\n    }\n\n";
		}
		output << "\n";
	}
	output.close();




}