#pragma once
#include <string>
class Exception {
	std::string msg;
public:
	Exception(std::string msg) : msg(msg) {}
	std::string getMsg() { return msg; }
};