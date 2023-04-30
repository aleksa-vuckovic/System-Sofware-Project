#pragma once
#include "Instruction.h"
#include <iostream>
Instruction::Instruction(std::string mne, Operand* ops[3]) : mnemonic(mne) {
	for (int i = 0; i < 3; i++) this->ops[i] = ops[i];
}
Operand* Instruction::getOperand(int i) {
	return ops[i];
}
int Instruction::getOperandCount() {
	int cnt = 0;
	for (int i = 0; i < cnt; i++) if (ops[i]) cnt++;
	return cnt;
}
Instruction::~Instruction() {
	for (int i = 0; i < 3; i++) if (ops[i]) {
		delete ops[i];
		ops[i] = nullptr;
	}
}
std::string Instruction::getMnemonic() {
	return mnemonic;
}
std::string Instruction::str() {
	return mnemonic + " " + (ops[0] ? ops[0]->str() + (ops[1] ? ", " + ops[1]->str() + (ops[2] ? ", " + ops[2]->str() : "") : "") : "");
}
Directive::Directive(std::string name, std::string args) : name(name), args(args) {}
std::string Directive::str() {
	return "." + name + " " + args;
}