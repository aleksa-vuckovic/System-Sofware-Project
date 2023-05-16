#include "Operand.h"
std::string Operand::getSymbol() {
	return "";
}
Operand::~Operand() {}
void Operand::freeOperandList(std::vector<Operand*>* list) {
	for (auto i = list->begin(); i != list->end(); i++) delete* i;
	delete list;
}
LiteralOperand::LiteralOperand(long long val) : val(val) {}
long long LiteralOperand::getLiteral() {
	return val;
}
std::string  LiteralOperand::str() {
	return std::to_string(val);
}
ImmediateLiteralOperand::ImmediateLiteralOperand(long long val) : LiteralOperand(val) {}
Operand::Type ImmediateLiteralOperand::getType() {
	return IMM_LIT;
}
std::string ImmediateLiteralOperand::str() {
	return "$" + LiteralOperand::str();
}
MemoryLiteralOperand::MemoryLiteralOperand(long long val) : LiteralOperand(val) {}
Operand::Type MemoryLiteralOperand::getType() {
	return MEM_LIT;
}
SymbolOperand::SymbolOperand(std::string sym) : sym(sym) {}
std::string SymbolOperand::getSymbol() {
	return sym;
}
std::string SymbolOperand::str() {
	return sym;
}
ImmediateSymbolOperand::ImmediateSymbolOperand(std::string sym) : SymbolOperand(sym) {}
Operand::Type ImmediateSymbolOperand::getType() {
	return IMM_SYM;
}
std::string ImmediateSymbolOperand::str() {
	return "$" + SymbolOperand::str();
}
MemorySymbolOperand::MemorySymbolOperand(std::string sym) : SymbolOperand(sym) {}
Operand::Type MemorySymbolOperand::getType() {
	return MEM_SYM;
}
RegisterOperand::RegisterOperand(int reg) : reg(reg) {}
int RegisterOperand::getRegister() {
	return reg;
}
std::string RegisterOperand::str() {
	return "%r" + std::to_string(reg);
}
DirectRegisterOperand::DirectRegisterOperand(int reg) : RegisterOperand(reg) {}
Operand::Type DirectRegisterOperand::getType() {
	return REG_DIR;
}
IndirectRegisterOperand::IndirectRegisterOperand(int reg) : RegisterOperand(reg) {}
Operand::Type IndirectRegisterOperand::getType() {
	return REG_IND;
}
std::string IndirectRegisterOperand::str() {
	return "[" + RegisterOperand::str() + "]";
}
LiteralRegisterOperand::LiteralRegisterOperand(long long val, int reg) : RegisterOperand(reg), LiteralOperand(val) {}
Operand::Type LiteralRegisterOperand::getType() {
	return REG_LIT;
}
std::string LiteralRegisterOperand::str() {
	return "[" + RegisterOperand::str() + " + " + LiteralOperand::str() + "]";
}
SymbolRegisterOperand::SymbolRegisterOperand(std::string sym, int reg) : RegisterOperand(reg), SymbolOperand(sym) {}
Operand::Type SymbolRegisterOperand::getType() {
	return REG_SYM;
}
std::string SymbolRegisterOperand::str() {
	return "[" + RegisterOperand::str() + " + " + SymbolOperand::str() + "]";
}