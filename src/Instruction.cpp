#include "../inc/Instruction.h"
#include <iostream>
Instruction::Instruction(std::string mne, std::vector<Operand*>* ops) : mnemonic(mne), operands(ops) {}
Operand* Instruction::getOperand(int i) {
	return operands->at(i);
}
int Instruction::getOperandCount() {
	return operands->size();
}
Instruction::~Instruction() {
	Operand::freeOperandList(operands);
	operands = nullptr;
}
std::string Instruction::getMnemonic() {
	return mnemonic;
}
std::string Instruction::str() {
	std::string res = mnemonic;
	for (int i = 0; i < operands->size(); i++) {
		if (i == 0) res += " ";
		else res += ", ";
		res += operands->at(i)->str();
	}
	return res;
}

Directive::Directive(std::string name, std::vector<Operand*>* args) {
	this->name = name;
	this->args = args;
}
Directive::~Directive() {
	Operand::freeOperandList(args);
	args = nullptr;
}
Operand* Directive::getOperand(int i) {
	return args->at(i);
}
int Directive::getOperandCount() {
	return args->size();
}
std::string Directive::getName() {
	return name;
}
std::string Directive::str() {
	std::string res = "." + name + " ";
	for (auto it = args->begin(); it != args->end();) {
		res += (*it)->str();
		it++;
		if (it != args->end()) res += ", ";
	}
	return res;
}