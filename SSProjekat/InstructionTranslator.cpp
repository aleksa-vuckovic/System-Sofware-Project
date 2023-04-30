#include "InstructionTranslator.h"
#include "Converter.h"

std::string InstructionTranslator::mnemonics[] = {
		"halt", "int", "iret", "call", "ret", "jmp", "beq", "bne",
		"bgt", "push", "pop", "xchg", "add", "sub", "mul", "div",
		"not", "and", "or", "xor", "shl", "shr", "ld"
};
void InstructionTranslator::checkMnemonic(Instruction* ins) {
	bool ok = false;
	for (int i = 0; i < sizeof(mnemonics) / sizeof(std::string)) {
		if (mnemonics[i] == ins->getMnemonic()) ok = true;
	}
	if (!ok) throw TranslationException("InstructionTranslator::TranslationException: Unrecognized mnemonic.");
}
void InstructionTranslator::checkOpNum(Instruction* ins, int num) {
	if (ins->getOperandCount() != num) throw TranslationException("InstructionTranslator::TranslationException: Unexpected operand count.");
}
void InstructionTranslator::checkOpTypeReg(Operand* op) {
	if (op->getType() != Operand::REG_DIR) throw TranslationException("InstructionTranslator::TranslationException: Unexpected operand type - expected REG_DIR.");
}
void InstructionTranslator::checkOpTypeJmp(Operand* op) {
	Operand::Type t = op->getType();
	if (t != Operand::MEM_LIT || t != Operand::MEM_SYM) throw TranslationException("InstructionTranslator::TranslationException: Unexpected operand type for a jmp/branch instruction.");
}
void InstructionTranslator::checkOpTypeSt(Operand* op) {
	Operand::Type t = op->getType();
	if (t == Operand::IMM_LIT || t == Operand::MEM_LIT) throw TranslationException("InstructionTranslator::TranslationException: Unexpected operand type for a store instruction.");
}
void InstructionTranslator::checkGPR(Operand* op) {
	if (!op->hasRegister()) return;
	int t = op->getRegister();
	if (t < GPR_MIN || t > GPR_MAX) throw TranslationException("InstructionTranslator::TranslationException: GP register number out of range.");
}
void InstructionTranslator::checkCSR(Operand* op) {
	if (!op->hasRegister()) return;
	int t = op->getRegister();
	if (t < CSR_MIN || t > CSR_MAX) throw TranslationException("InstructionTranslator::TranslationException: CS register number out of range.");
}
void InstructionTranslator::checkLiteral(Operand* op) {
	if (!op->hasLiteral()) return;
	Operand::Type t = op->getType();
	long min, max;
	if (t == Operand::IMM_LIT || t == Operand::REG_LIT) min = IMM_LIT_MIN, max = IMM_LIT_MAX;
	else min = MEM_LIT_MIN, max = MEM_LIT_MAX;
	long lit = op->getLiteral();
	if (lit < min || lit > max) throw TranslationException("InstructionTranslator::TranslationException: Literal is out of range.");
}

void InstructionTranslator::checkInstruction(Instruction* ins) {
	checkMnemonic(ins);
	std::string m = ins->getMnemonic();
	//Operand count check
	if (m == "halt" || m == "int" || m == "iret" || m == "ret") checkOpNum(ins, 0);
	else if (m == "jmp" || m == "push" || m == "pop" || m == "not") checkOpNum(ins, 1);
	else if (m == "beq" || m == "bne" || m == "bgt") checkOpNum(ins, 3);
	else checkOpNum(ins, 2);

	if (ins->getOperandCount() == 0) return;
	//Checking first operand
	Operand* op = ins->getOperand(0);
	if (m == "call" || m == "jmp") checkOpTypeJmp(op);
	else if (m == "ld");
	else checkOpTypeReg(op);

	if (m == "csrrd") checkCSR(op);
	else checkGPR(op);
	checkLiteral(op);

	if (ins->getOperandCount() == 1) return;
	//Checking second operand
	op = ins->getOperand(1);
	if (m == "st") checkOpTypeSt(op);
	else checkOpTypeReg(op);

	if (m == "csrwr") checkCSR(op);
	else checkGPR(op);
	checkLiteral(op);

	if (ins->getOperandCount() == 2) return;
	//Checking third operand
	op = ins->getOperand(2);
	checkOpTypeJmp(op);
	checkGPR(op);
	checkLiteral(op);
}
int InstructionTranslator::getSize(Instruction* ins) {
	std::string m = ins->getMnemonic();
	if (m == "iret" || m == "call") return 24;
	if (m == "ret" || m == "push") return 16;
	if (m == "ld") {
		Operand* op = ins->getOperand(1);
		Operand::Type t = op->getType();
		if (t == Operand::MEM_LIT || t == Operand::MEM_SYM
			|| t == Operand::REG_DIR || t == Operand::REG_IND) return 16;
		else return 8;
	}
	if (m == "st") {
		Operand* op = ins->getOperand(1);
		Operand::Type t = op->getType();
		if (t == Operand::REG_DIR || t == Operand::REG_IND) return 8;
		else return 40;
	}
	return 8;
}

std::string InstructionTranslator::addsp(int num) {
	return "91EE0" + Converter::toHex(num, 3);
}
std::string InstructionTranslator::push(int reg) {
	return std::string("91EE0FFC80E0") + Converter::hexChar(reg) + "as";
}
std::string InstructionTranslator::pop(int reg) {
	return std::string("93") + Converter::hexChar(reg) + "E0004";
}

std::string InstructionTranslator::translate(Instruction* ins, int* litoffset, int* reloffset) {
	*litoffset = -1; *reloffset = -1;
	std::string m = ins->getMnemonic();
	if (m == "halt") return "00000000";
	if (m == "int") return "10000000";
	if (m == "iret") {
		return addsp(8) + "960E0FFC92FE0FF8";
	}
	if (m == "call") {
		*reloffset = 21;
		return push(15) + "30F00000";
	}
	if (m == "ret") {
		return addsp(4) + "92FE9FFC";
	}
	if (m == "jmp") {
		*reloffset = 5;
		return "30F00000";
	}
	if (m == "beq" || m == "bne" || m == "bgt") {
		std::string code;
		if (m == "beq") code = "31";
		else if (m == "bne") code = "32";
		else code = "33";
		*reloffset = 5;
		int regA = ins->getOperand(0)->getRegister();
		int regB = ins->getOperand(1)->getRegister();
		return code + "F" + Converter::hexChar(regA) + Converter::hexChar(regB) + "000";
	}
	if (m == "push") {
		return push(ins->getOperand(0)->getRegister());
	}
	if (m == "pop") {
		return pop(ins->getOperand(0)->getRegister());
	}
	if (m == "xchg") {
		int regA = ins->getOperand(0)->getRegister();
		int regB = ins->getOperand(1)->getRegister();
		return std::string("400") + Converter::hexChar(regA) + Converter::hexChar(regB) + "000";
	}
	if (m == "add" || m == "sub" || m == "mul" || m == "div") {
		std::string code;
		if (m == "add") code = "50";
		else if (m == "sub") code = "51";
		else if (m == "mul") code = "52";
		else code = "53";
		int regA = ins->getOperand(0)->getRegister();
		int regB = ins->getOperand(1)->getRegister();
		return code + Converter::hexChar(regB) + Converter::hexChar(regB) + Converter::hexChar(regA) + "000";
	}
	if (m == "not") {
		char reg = Converter::hexChar(ins->getOperand(0)->getRegister());
		return std::string("60") + reg + reg + "0000";
	}
	if (m == "and" || m == "or" || m == "xor") {
		std::string code;
		if (m == "and") code = "61";
		else if (m == "or") code = "62";
		else code = "63";
		int regA = ins->getOperand(0)->getRegister();
		int regB = ins->getOperand(1)->getRegister();
		return code + Converter::hexChar(regB) + Converter::hexChar(regB) + Converter::hexChar(regA) + "000";
	}
	if (m == "shl" || m == "shr") {
		std::string code = m == "shl" ? "70" : "71";
		int regA = ins->getOperand(0)->getRegister();
		int regB = ins->getOperand(1)->getRegister();
		return code + Converter::hexChar(regB) + Converter::hexChar(regB) + Converter::hexChar(regA) + "000";
	}
	if (m == "ld") {
		Operand* op = ins->getOperand(0);
		Operand::Type t = op->getType();
		char regA = Converter::hexChar(ins->getOperand(1)->getRegister());
		if (t == Operand::IMM_LIT || t == Operand::IMM_SYM) {
			*litoffset = 5;
			return std::string("92") + regA +"F0000";
		}
		if (t == Operand::MEM_LIT || t == Operand::MEM_SYM) {
			*litoffset = 5;
			return std::string("92") + regA + "F000092AA0000";
		}
		if (t == Operand::REG_DIR) {
			char regB = Converter::hexChar(op->getRegister());
			return std::string("01") + regA + regB + "0000";
		}
		else {
			*litoffset = 5;
			char regB = Converter::hexChar(op->getRegister());
			return std::string("92") + regA + "F000092" + regA + regA + regB + "000";
		}
	}
	if (m == "st") {
		Operand* op = ins->getOperand(1);
		Operand::Type t = op->getType();
		char regA = Converter::hexChar(ins->getOperand(0)->getRegister());
		if (t == Operand::MEM_LIT || t == Operand::MEM_SYM) {
			char regC = regA == '1' ? '2' : '1';
			*litoffset = 21;
			return push(regC - '0') + "92" + regC + "F000080" + regC + "0" + regA + "000" + pop(regC - '0');
		}
		char regB = Converter::hexChar(ins->getOperand(1)->getRegister());
		if (t == Operand::REG_DIR) {
			return std::string("91") + regB + regA + "0000";
		}
		if (t == Operand::REG_IND) {
			return std::string("80") + regB + "0" + regA + "000";
		}
		else {
			char regC = regA == '1' ? '2' : '1';
			*litoffset = 21;
			return push(regC - '0') + "92" + regC + "F000080" + regB + regC + regA + "000" + pop(regC - '0');
		}
	}
	else {
		char regA = ins->getOperand(0)->getRegister();
		char regB = ins->getOperand(1)->getRegister();
		return std::string(m == "csrrd" ? "90" : "91") + regB + regA + "0000";
	}
}