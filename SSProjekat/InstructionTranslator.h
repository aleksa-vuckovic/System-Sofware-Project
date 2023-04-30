#ifndef _INSTRUCTIONTRANSLATOR_H_
#define _INSTRUCTIONTRANSLATOR_H_
#include "Instruction.h"
class InstructionTranslator {
	int getSize(Instruction* ins);
	std::string translate(Instruction* ins);
};

#endif