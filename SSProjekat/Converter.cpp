#include "Converter.h"

std::string Converter::toHex(long lit) {
	if (lit < 0) {
		lit = 1l << 8 + lit;
	}
	std::string res = "12345678";
	for (int i = 7; i >= 0; i--) {
		int dig = lit % 16;
		res[i] = hexChar(dig);
		lit /= 16;
	}
	return res;
}

char Converter::hexChar(int dig) {
	if (dig < 10) return '0' + (char)dig;
	return 'A' + (char)(dig - 10);
}

std::string Converter::toLower(std::string s) {
	std::string res = s;
	for (int i = 0; i < s.length(); i++) res[i] = std::tolower(s[i]);
	return res;
}
bool Converter::isBlank(char c) {
	return c == ' ' || c == '\t' || c == '\n';
}
std::string Converter::removeBlanks(std::string s) {
	std::string res = "";
	for (int i = 0; i < s.length(); i++) if (!isBlank(s[i])) res += s[i];
	return res;
}