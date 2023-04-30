#pragma once
#include "Operand.h"
class Instruction {
	std::string mnemonic;
	Operand* ops[3];
public:
	Instruction(std::string mne, Operand* ops[3]);
	~Instruction();
	std::string getMnemonic();
	std::string str();
};
class Directive {
public:
	std::string name;
	std::string args;
	Directive(std::string name, std::string args);
	std::string str();
};