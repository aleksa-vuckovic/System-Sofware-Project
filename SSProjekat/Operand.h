#ifndef _OPERAND_H_
#define _OPERAND_H_
#include <string>
#include <vector>
class Operand {
public:
	typedef enum Type { IMM_LIT, IMM_SYM, MEM_LIT, MEM_SYM, REG_DIR, REG_IND, REG_LIT, REG_SYM } Type;
	virtual std::string getSymbol();
	virtual long long getLiteral() { return 0; }
	virtual int getRegister() { return 0; }
	virtual Type getType() = 0;

	virtual bool hasLiteral() { return false; }
	virtual bool hasRegister() { return false;  }
	virtual bool hasSymbol() { return false; }
	virtual std::string str() = 0;
	virtual ~Operand();

	static void freeOperandList(std::vector<Operand*>* list);
};
class LiteralOperand : public virtual Operand {
	long long val;
public:
	LiteralOperand(long long val);
	long long getLiteral() override;
	bool hasLiteral() override { return true; }
	std::string str();
};
class ImmediateLiteralOperand : public LiteralOperand {
public:
	ImmediateLiteralOperand(long long val);
	Type getType();
	std::string str();
};
class MemoryLiteralOperand : public LiteralOperand {
public:
	MemoryLiteralOperand(long long val);
	Type getType();
};
class SymbolOperand : public virtual Operand {
	std::string sym;
public:
	SymbolOperand(std::string sym);
	std::string getSymbol();
	std::string str();
	bool hasSymbol() override { return true; }
};
class ImmediateSymbolOperand : public SymbolOperand {
public:
	ImmediateSymbolOperand(std::string sym);
	Type getType();
	std::string str();
};
class MemorySymbolOperand : public SymbolOperand {
public:
	MemorySymbolOperand(std::string sym);
	Type getType();
};
class RegisterOperand: public virtual Operand {
	int reg;
public:
	RegisterOperand(int reg);
	int getRegister();
	std::string str();
};
class DirectRegisterOperand : public RegisterOperand {
public:
	DirectRegisterOperand(int reg);
	Type getType();
};
class IndirectRegisterOperand : public RegisterOperand {
public:
	IndirectRegisterOperand(int reg);
	Type getType();
	std::string str();
};
class LiteralRegisterOperand : public RegisterOperand, public LiteralOperand {
public:
	LiteralRegisterOperand(long long val, int reg);
	Type getType();
	std::string str();
};
class SymbolRegisterOperand : public RegisterOperand, public SymbolOperand {
public:
	SymbolRegisterOperand(std::string sym, int reg);
	Type getType();
	std::string str();
};
#endif