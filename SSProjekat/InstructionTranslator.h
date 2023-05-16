#ifndef _INSTRUCTIONTRANSLATOR_H_
#define _INSTRUCTIONTRANSLATOR_H_
#include "Instruction.h"
#include "Exception.h"
#define GPR_MIN 0
#define GPR_MAX 15
#define CSR_MIN 0
#define CSR_MAX 2
#define IMM_LIT_MIN -(1ll << 31)
#define IMM_LIT_MAX 0xFFFFFFFFll
#define MEM_LIT_MIN 0
#define MEM_LIT_MAX IMM_LIT_MAX

class InstructionTranslator {
	class TranslationException : public Exception {
	public:
		TranslationException(std::string msg) : Exception(msg) {}
	};
	static std::string mnemonics[];
	void checkMnemonic(Instruction* ins);
	void checkOpNum(Instruction* ins, int num);
	void checkOpTypeReg(Operand* op);
	void checkOpTypeJmp(Operand* op);
	void checkOpTypeSt(Operand* op);
	void checkGPR(Operand* op);
	void checkCSR(Operand* op);
	void checkLiteral(Operand* op);

	std::string addsp(int num);
	std::string push(int reg);
	std::string pop(int reg);
public:
	/*
	Check all of the following requirements :
		1. Number of operands
		2. Mnemonic
		3. Operand type
		4. Register number - 0-15 for gpr, 0-2 for csr instructions
		5. literal - 32 bit postive for memory, 32bit positive or negative for immediate
	*/
	void checkInstruction(Instruction* ins);
	//It is assumed that the instruction has been checked by calling checkingInstruction, before
	// all of the following methods
	//returns the size IN BYTES (which is 2x less than the number of chars)
	int getSize(Instruction* ins);
	//litoffset is the offset in chars, relative to the start of the translated instruction, where the offset to the literal pool is to be inserted
	//reloffset is the offset in chars, relative to the start of the translated instruction, where the location that requires a pc relative relocation is
	std::string translate(Instruction* ins, int* litoffset, int* reloffset);


	void checkDirective(Directive* dir);
	int getSize(Directive* dir);
};

#endif