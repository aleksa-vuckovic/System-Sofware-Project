#include "Converter.h"

std::string Converter::toHex32(long long lit) {
	return toHex(lit, 8);
}
std::string Converter::toHex(long long num, int digs) {
	if (num < 0) num = (1ll << digs*4) + num;
	std::string res = "";
	for (int i = digs - 1; i >= 0; i--) {
		int dig = num % 16;
		res = hexChar(dig) + res;
		num /= 16;
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
std::string Converter::toLittleEndian(std::string val)
{
	if (val.length() % 2) val = "0" + val;
	std::string res = "";
	for (int i = 0; i < val.length(); i+=2) {
		res = val.substr(i, 2) + res;
	}
	return res;
}
std::string Converter::removeBlanks(std::string s) {
	std::string res = "";
	for (int i = 0; i < s.length(); i++) if (!isBlank(s[i])) res += s[i];
	return res;
}