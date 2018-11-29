#include "Math.hpp"
#include <algorithm>
#include <iostream>
#include <algorithm>
#include <cctype>

namespace Math{
    /*
        Returns hex form of given integer
    */
    std::string decHex(int num, unsigned int size){
         if(num == 0){
			 return std::string(size, '0');
         }
         std::string text = "";
         int b = num, current = 0, a = 0;
         do{
             a = b % 16;
             if(a < 10){
                text += char(a+48);
             }else{
                text += char(a+48+7);
             }
             b /= 16;
             current++;
         }while(b != 0);
         while(text.size() != size){
            text.append(1, '0');
         }
         std::reverse(text.begin(), text.end());
         return (text.size() <= 0) ? "?" : text;
     }

	std::string decHexLower(int num, unsigned int size) {
		if (num == 0) {
			return std::string(size, '0');
		}
		std::string text = "";
		int b = num, current = 0, a = 0;
		do {
			a = b % 16;
			if (a < 10) {
				text += char(a + 48);
			}
			else {
				text += char(a + 48 + 7);
			}
			b /= 16;
			current++;
		} while (b != 0);
		while (text.size() != size) {
			text.append(1, '0');
		}
		std::reverse(text.begin(), text.end());
		std::transform(text.begin(), text.end(), text.begin(), tolower);
		return (text.size() <= 0) ? "?" : text;
	}

     /*
        Because i'm a lazy fuck
     */
     std::string decBin(int num){
          std::string text = "";
         int b = num, a = 0;
         do{
             a = b % 2;
             if(a < 10){
                text += char(a+48);
             }else{
                text += char(a+48+7);
             }
             b /= 2;
         }while(b != 0);
         while(text.size() != 4){
            text.append(1, '0');
         }
         std::reverse(text.begin(), text.end());
         return (text.size() <= 0) ? "?" : text;
       
     }
}
