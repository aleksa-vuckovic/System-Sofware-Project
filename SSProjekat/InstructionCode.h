#ifndef _INSTRUCTION_CODE_
#define _INSTRUCTION_CODE_
#include <string>

/*
* This class represents a single machine code instruction.
* The code passed to the constructor is obtained by reading the
* instruction from memory as a little endian integer.
*/
class InstructionCode {
	unsigned code;
	int fromHexChar(char c);
public:
	InstructionCode(unsigned code);
	int getOC();
	int getA();
	int getB();
	int getC();
	int getDisplacement();
};

#endif