#include "../inc/InstructionCode.h"

InstructionCode::InstructionCode(unsigned code) : code(code) {}
//OC AB CD DD
//DD CD AB OC
int InstructionCode::fromHexChar(char c) {
	if (std::isalpha(c)) return 10 + c - (std::isupper(c) ? 'A' : 'a');
}
int InstructionCode::getOC() {
	return (code & 0xff);
}
int InstructionCode::getA() {
	return (code & 0xf000) >> 12;
}
int InstructionCode::getB() {
	return (code & 0xf00) >> 8;
}
int InstructionCode::getC() {
	return (code & 0xf00000) >> 20;
}
int InstructionCode::getDisplacement() {
	int d = ((code & 0xff000000) >> 24) + ((code & 0xf0000) >> 8);
	if (d > (1 << 11)) d = d - (1 << 12);
	return d;
}