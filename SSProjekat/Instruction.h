#ifndef _INSTRUCTION_H_
#define _INSTRUCTION_H_
#include "Operand.h"
#include <vector>
class Instruction {
	std::string mnemonic;
	std::vector<Operand*>* operands;
public:
	Instruction(std::string mne, std::vector<Operand*>* operands);
	~Instruction();
	std::string getMnemonic();
	std::string str();
	Operand* getOperand(int i);
	int getOperandCount();
};
class Directive {
	std::string name;
	std::vector<Operand*>* args;
public:
	Directive(std::string name, std::vector<Operand*>* args);
	~Directive();
	Operand* getOperand(int i);
	int getOperandCount();
	std::string getName();
	std::string str();
};
#endif