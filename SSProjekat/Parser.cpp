#include "Parser.h"
#include "Instruction.h"
#include <regex>
#include <cctype>
#include "Operand.h"
#include "Converter.h"

Parser::Parser() : directivePattern(R"del(^\s*(?:(\w+):)?\s*\.(\w+)\s+(.*)$)del"),
		instructionPattern(R"del(^\s*(?:(\w+):)?\s*(\w{2,4})\s*(.*)$)del"),
		labelOnlyPattern(R"del(^\s*(\w+):\s*$)del"),
		operatorListPattern(R"del(^([^,]+)(?:,([^,]+)(?:,([^,]+))?)?$)del"),
		IMM_LITPattern(R"del(^\s*\$([+-]?[0-9]\w*)\s*$)del"),
		IMM_SYMPattern(R"del(^\s*\$([a-zA-Z]\w*)\s*$)del"),
		MEM_LITPattern(R"del(^\s*([0-9]\w*)\s*$)del"),
		MEM_SYMPattern(R"del(^\s*([a-zA-Z]\w*)\s*$)del"),
		REG_DIRPattern(R"del(^\s*%(\w+)\s*$)del"),
		REG_INDPattern(R"del(^\s*\[\s*%(\w+)\s*\]\s*$)del"),
		REG_LITPattern(R"del(^\s*\[\s*%([a-zA-Z0-9]+)\s*([+-]\s*[0-9]\w*)\s*\]\s*$)del"),
		REG_SYMPattern(R"del(^\s*\[\s*%([a-zA-Z0-9]+)\s*\+\s*([a-zA-Z]\w*)\s*\]\s*$)del"),
		HEX_LITPattern(R"del(^[+-]0x[0-9a-f]+$)del") {}
long Parser::parseLiteral(std::string lit) {
	long res;
	try {
		lit = Converter::toLower(Converter::removeBlanks(lit));
		if (std::regex_match(lit, HEX_LITPattern))
			res = std::stol(lit, nullptr, 16);
		else res = std::stol(lit);
	}
	catch (std::exception e) {
		throw OperatorException("Parser::OperatorException: Incorrect literal value!");
	}
	return res;
}
int Parser::parseRegister(std::string reg) {
	reg = Converter::toLower(reg);
	if (reg == "status") return 0;
	if (reg == "handler") return 1;
	if (reg == "cause") return 2;
	if (reg == "sp") return 14;
	if (reg == "pc") return 15;
	if (reg[0] != 'r') throw OperatorException("Parser::OperatorException: Incorrect register operator!");
	std::string num = reg.substr(1);
	int res;
	try {
		res = std::stoi(num);
	}
	catch (std::exception e) {
		throw OperatorException("Parser::OperatorException: Incorrect register operator number!");
	}
	if (res < 0 || res > 15) throw OperatorException("Parser::OperatorException: Incorrect register operator number!");
	return res;
}
Operand* Parser::parseOperator(std::string operand) {
	if (operand == "") return nullptr;
	std::sregex_iterator end;
	std::sregex_iterator iter = std::sregex_iterator(operand.begin(), operand.end(), IMM_LITPattern);
	if (iter != end) {
		std::smatch match = *iter;
		return new ImmediateLiteralOperand(parseLiteral(match[1].str()));
	}
	iter = std::sregex_iterator(operand.begin(), operand.end(), IMM_SYMPattern);
	if (iter != end) {
		std::smatch match = *iter;
		return new ImmediateSymbolOperand(match[1].str());
	}
	iter = std::sregex_iterator(operand.begin(), operand.end(), MEM_LITPattern);
	if (iter != end) {
		std::smatch match = *iter;
		return new MemoryLiteralOperand(parseLiteral(match[1].str()));
	}
	iter = std::sregex_iterator(operand.begin(), operand.end(), MEM_SYMPattern);
	if (iter != end) {
		std::smatch match = *iter;
		return new MemorySymbolOperand(match[1].str());
	}
	iter = std::sregex_iterator(operand.begin(), operand.end(), REG_DIRPattern);
	if (iter != end) {
		std::smatch match = *iter;
		int reg = parseRegister(match[1].str());
		return new DirectRegisterOperand(reg);
	}
	iter = std::sregex_iterator(operand.begin(), operand.end(), REG_INDPattern);
	if (iter != end) {
		std::smatch match = *iter;
		int reg = parseRegister(match[1].str());
		return new IndirectRegisterOperand(reg);
	}
	iter = std::sregex_iterator(operand.begin(), operand.end(), REG_LITPattern);
	if (iter != end) {
		std::smatch match = *iter;
		int reg = parseRegister(match[1].str());
		int lit = parseLiteral(match[2].str());
		return new LiteralRegisterOperand(lit, reg);
	}
	iter = std::sregex_iterator(operand.begin(), operand.end(), REG_SYMPattern);
	if (iter != end) {
		std::smatch match = *iter;
		int reg = parseRegister(match[1].str());
		std::string sym = match[2].str();
		return new SymbolRegisterOperand(sym, reg);
	}
	throw OperatorException("Parser::OperatorException: Invalid operand expression: " + operand);
}
void Parser::parseAssemblerLine(std::string line, std::string* label, Directive** dirp, Instruction** insp) {
	std::sregex_iterator iter(line.begin(), line.end(), directivePattern);
	std::sregex_iterator end;

	iter = std::sregex_iterator(line.begin(), line.end(), labelOnlyPattern);
	if (iter != end) {
		std::smatch match = *iter;
		*label = match[1].str();
		*dirp = nullptr;
		*insp = nullptr;
		return;
	}

	iter = std::sregex_iterator(line.begin(), line.end(), directivePattern);
	if (iter != end) {
		//directive
		std::smatch match = *iter;
		*label = match[1].str();
		*dirp = new Directive(match[2].str(), match[3].str());
		*insp = nullptr;
		return;
	}

	iter = std::sregex_iterator(line.begin(), line.end(), instructionPattern);
	if (iter != end) {
		//instruction
		std::smatch match = *iter;
		*label = match[1].str();
		std::string mnemonic = Converter::toLower(match[2].str());

		//parsing the operators
		std::string operatorList = match[3];
		iter = std::sregex_iterator(operatorList.begin(), operatorList.end(), operatorListPattern);
		std::string operators[3] = { "", "", "" };
		if (iter != end) {
			std::smatch match = *iter;
			for (int i = 0; i < 3; i++) operators[i] = match[i + 1].str();
		}
		Operand* ops[3];
		for (int i = 0; i < 3; i++) ops[i] = parseOperator(operators[i]);
		*insp = new Instruction(mnemonic, ops);
		*dirp = nullptr;
		return;
	}

	throw ParserException("Parser::ParserException: Incorrect assembly syntax!");
}
