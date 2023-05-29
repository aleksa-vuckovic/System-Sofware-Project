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


/*
* Used for checking correctness, calculating the size of, and translating instructions.
*/
class InstructionTranslator {
	class TranslationException : public Exception {
	public:
		TranslationException(std::string msg) : Exception(msg) {}
	};
	//Allowed mnemonics, in lowercase.
	static std::string mnemonics[];
	void checkMnemonic(Instruction* ins);
	//Checks if the number of operands is correct for the mnemonic.
	void checkOpNum(Instruction* ins, int num);
	//Checks if the operand is of type REG_DIR.
	void checkOpTypeReg(Operand* op);
	//Checks if the operand is of type MEM_LIT, or MEM_SYM (the allowed types for jmp and branch instructions).
	void checkOpTypeJmp(Operand* op);
	//Checks the operand type for store instruction (IMM_LIT, IMM_SYM and SPEC are not allowed).
	void checkOpTypeSt(Operand* op);
	//Checks the operand type for ld instruction (SPEC is not allowed).
	void checkOpTypeLd(Operand* op);
	//Checks if the register number is a valid GPR register, if the operand has a register number.
	void checkGPR(Operand* op);
	//Checks if the register number is a valid CSR register, if the operand has a register number.
	void checkCSR(Operand* op);
	//Checks if the literal is within bounds for the given operand type.
	void checkLiteral(Operand* op);

	std::string push(int reg);
	std::string pop(int reg);
public:
	/*
	* Check all of the following requirements :
	* 	1. Number of operands
	* 	2. Mnemonic
	* 	3. Operand type
	* 	4. Register number - 0-15 for gpr, 0-2 for csr instructions
	* 	5. literal - 32 bit positive for memory, 32 bit positive or negative for immediate
	*/
	void checkInstruction(Instruction* ins);
	//It is assumed that the instruction has been checked by calling checkInstruction, before calling the following two methods
	
	//Returns the size IN BYTES (which is 2x less than the number of chars).
	int getSize(Instruction* ins);
	
	/*
	* Translates the instruction and returns the resulting string. If any fixups are needed, litoffset and reloffset are set.
	* Litoffset is the offset in chars, relative to the start of the translated instruction, where the offset to the literal pool is to be inserted.
	* Reloffset is the offset in chars, relative to the start of the translated instruction, where the location that requires a pc relative relocation is.
	*/
	std::string translate(Instruction* ins, int* litoffset, int* reloffset);


	//Checks the number and type of operands of a directive.
	void checkDirective(Directive* dir);
	//Returns the size of the code resulting from a directive.
	int getSize(Directive* dir);
};

#endif