#ifndef _OPERAND_H_
#define _OPERAND_H_
#include <string>
#include <vector>
/*
* Represents one operand from an assembly instruction.
* There are 8 types of operands: Immediate Literal, Immediate Symbol, Memory Literal, Memory Symbol, Register Direct, Register Indirect, Register Literal, Register Symbol.
* Each type has its own class.
* There is also the Special operand type which was added for the .ascii and .equ directive,
* which is meant to retain the full operand string, as provided in the assembly file.
*/
class Operand {
public:
	typedef enum Type { IMM_LIT, IMM_SYM, MEM_LIT, MEM_SYM, REG_DIR, REG_IND, REG_LIT, REG_SYM, SPEC } Type;
	virtual std::string getSymbol() { return ""; }
	virtual long long getLiteral() { return 0; }
	virtual int getRegister() { return 0; }
	virtual std::string getOriginalString() { return ""; }
	virtual Type getType() = 0;

	virtual bool hasLiteral() { return false; }
	virtual bool hasRegister() { return false;  }
	virtual bool hasSymbol() { return false; }
	virtual std::string str() = 0;
	virtual ~Operand() {}

	//Deallocates all operands in a list. This method is used in other classes.
	static void freeOperandList(std::vector<Operand*>* list) {
		for (auto i = list->begin(); i != list->end(); i++) delete* i;
		delete list;
	}
};

class LiteralOperand : public virtual Operand {
	long long val;
public:
	LiteralOperand(long long val) : val(val) {}
	long long getLiteral() override { return val; }
	bool hasLiteral() override { return true; }
	std::string str() { return std::to_string(val); }
};

class ImmediateLiteralOperand : public LiteralOperand {
public:
	ImmediateLiteralOperand(long long val) : LiteralOperand(val) {}
	Type getType() { return IMM_LIT; }
	std::string str() { return "$" + LiteralOperand::str(); }
};

class MemoryLiteralOperand : public LiteralOperand {
public:
	MemoryLiteralOperand(long long val) : LiteralOperand(val) {}
	Type getType() { return MEM_LIT; }
};
class SymbolOperand : public virtual Operand {
	std::string sym;
public:
	SymbolOperand(std::string sym) : sym(sym) {}
	std::string getSymbol() { return sym; }
	std::string str() { return sym; }
	bool hasSymbol() override { return true; }
};
class ImmediateSymbolOperand : public SymbolOperand {
public:
	ImmediateSymbolOperand(std::string sym) : SymbolOperand(sym) {}
	Type getType() { return IMM_SYM; }
	std::string str() { return "$" + SymbolOperand::str(); }
};
class MemorySymbolOperand : public SymbolOperand {
public:
	MemorySymbolOperand(std::string sym) : SymbolOperand(sym) {}
	Type getType() { return MEM_SYM; }
};
class RegisterOperand: public virtual Operand {
	int reg;
public:
	RegisterOperand(int reg) : reg(reg) {}
	int getRegister() { return reg; }
	std::string str() {return "%r" + std::to_string(reg); }
};
class DirectRegisterOperand : public RegisterOperand {
public:
	DirectRegisterOperand(int reg) : RegisterOperand(reg) {}
	Type getType() { return REG_DIR; }
};
class IndirectRegisterOperand : public RegisterOperand {
public:
	IndirectRegisterOperand(int reg) : RegisterOperand(reg) {}
	Type getType() { return REG_IND; }
	std::string str() { return "[" + RegisterOperand::str() + "]"; }
};
class LiteralRegisterOperand : public RegisterOperand, public LiteralOperand {
public:
	LiteralRegisterOperand(long long val, int reg) : RegisterOperand(reg), LiteralOperand(val) {}
	Type getType() { return REG_LIT; }
	std::string str() { return "[" + RegisterOperand::str() + " + " + LiteralOperand::str() + "]"; }
};
class SymbolRegisterOperand : public RegisterOperand, public SymbolOperand {
public:
	SymbolRegisterOperand(std::string sym, int reg) : RegisterOperand(reg), SymbolOperand(sym) {}
	Type getType() { return REG_SYM; }
	std::string str() { return "[" + RegisterOperand::str() + " + " + SymbolOperand::str() + "]"; }
};
class SpecialOperand : public Operand {
	std::string originalString;
public:
	SpecialOperand(std::string originalString) : originalString(originalString) {}
	Type getType() { return SPEC; }
	std::string getOriginalString() override { return originalString; }
	std::string str() { return originalString; }
};
#endif