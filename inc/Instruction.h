#ifndef _INSTRUCTION_H_
#define _INSTRUCTION_H_
#include "Operand.h"
#include <vector>
/*
* Represents one assembly instruction.
* Contains a mnemonic, and the list of operands, in the same order.
*/
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

/*
* Represents on directive.
* Contains the name, and the list of arguments, in the same order.
*/
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