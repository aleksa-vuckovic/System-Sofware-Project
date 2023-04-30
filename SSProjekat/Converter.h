#ifndef _CONVERTER_H_
#define _CONVERTER_H_
#include <string>

class Converter {
public:
	//returns a string of length exactly 8 (4 bytes)
	static std::string toHex32(long num);
	static std::string toHex(long num, int digs);
	static char hexChar(int dig);
	static std::string toLower(std::string s);
	static std::string removeBlanks(std::string s);
	static bool isBlank(char c);
};

#endif