#ifndef _CONVERTER_H_
#define _CONVERTER_H_
#include <string>

class Converter {
public:
	//Returns toHex(num, 8).
	static std::string toHex32(long long num);
	/*
	* Returns a string of exactly digs characters, by converting num into hexadecimal.
	* If digs characters is not enough for num, the result is undefined.
	*/
	static std::string toHex(long long num, int digs);
	/*
	* Returns the hexadecimal character corresponding to dig.
	* If dig is greater than 15 or smaller than 0 the result is undefined.
	*/
	static char hexChar(int dig);
	//Returns the string with all characters converted to lowercase.
	static std::string toLower(std::string s);
	//Returns string with all blank characters removed as defined by isBlank().
	static std::string removeBlanks(std::string s);
	//Returns true if c is '\n' or ' ' or '\t'
	static bool isBlank(char c);
	//Returns the string val, a hexadecimal value produced by toHex(), converted to little endian.
	static std::string toLittleEndian(std::string val);
};

#endif