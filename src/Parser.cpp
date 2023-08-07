#include "../inc/Parser.h"
#include "../inc/Instruction.h"
#include <regex>
#include <cctype>
#include "../inc/Operand.h"
#include "../inc/Converter.h"
#include <iostream>


std::string Parser::removeComment(std::string line) {
	std::regex pattern(R"del(([^#]*)(#.*)?)del");
	std::sregex_iterator iter = std::sregex_iterator(line.begin(), line.end(), pattern);
	std::sregex_iterator end;
	if (iter != end) {
		std::smatch match = *iter;
		return match[1].str();
	}
	return "";
}
bool Parser::emptyLine(std::string line) {
	std::regex pattern("^\\s*$");
	if (std::regex_match(line, pattern)) return true;
	else return false;
}
std::string Parser::unescape(std::string data) {
	std::string unescapedStr = "";
	bool escapeMode = false;
	for (char c : data) {
		if (escapeMode) {
			switch (c) {
			case 'n':
				unescapedStr += '\n';  // convert "\n" to newline character
				break;
			case 't':
				unescapedStr += '\t';  // convert "\t" to tab character
				break;
			case '\\':
				unescapedStr += '\\';
				break;
			case '0':
				unescapedStr += '\0';
				break;
			default:
				throw ParserException("Parser::ParserException: Unknown escape sequence.");
				break;
			}
			escapeMode = false;
		}
		else {
			if (c == '\\') {
				escapeMode = true;
			}
			else {
				unescapedStr += c;
			}
		}
	}
	return unescapedStr;
}
std::string Parser::parseAsciiString(std::string operand) {
	std::regex pattern(R"del([^"]*"([^"]*)".*)del");
	std::sregex_iterator iter = std::sregex_iterator(operand.begin(), operand.end(), pattern);
	std::sregex_iterator end;
	if (iter != end) {
		std::smatch match = *iter;
		return unescape(match[1].str());
	}
	return "";
}

Parser::Parser() : directivePattern(R"del(^\s*(?:(\w+):)?\s*\.(\w+)\s*(.*)$)del"),
		instructionPattern(R"del(^\s*(?:(\w+):)?\s*(\w{2,6})\s*(.*)$)del"),
		labelOnlyPattern(R"del(^\s*(\w+):\s*$)del"),
		commaSeparatorPattern(R"del(^([^,]*)(,.*)?$)del"),
		IMM_LITPattern(R"del(^\s*\$([+-]?[0-9]\w*)\s*$)del"),
		IMM_SYMPattern(R"del(^\s*\$([a-zA-Z\.]\w*)\s*$)del"),
		MEM_LITPattern(R"del(^\s*([+-]?[0-9]\w*)\s*$)del"),
		MEM_SYMPattern(R"del(^\s*([a-zA-Z\.]\w*)\s*$)del"),
		REG_DIRPattern(R"del(^\s*%(\w+)\s*$)del"),
		REG_INDPattern(R"del(^\s*\[\s*%(\w+)\s*\]\s*$)del"),
		REG_LITPattern(R"del(^\s*\[\s*%([a-zA-Z0-9]+)\s*([+-]\s*[0-9]\w*)\s*\]\s*$)del"),
		REG_SYMPattern(R"del(^\s*\[\s*%([a-zA-Z0-9]+)\s*\+\s*([a-zA-Z\.]\w*)\s*\]\s*$)del"),
		HEX_LITPattern(R"del(^[+-]?0x[0-9a-f]+$)del") {}
std::string Parser::getNextCSV(std::string& input) {
	std::sregex_iterator iter = std::sregex_iterator(input.begin(), input.end(), commaSeparatorPattern);
	std::sregex_iterator end;
	if (iter != end) {
		std::smatch match = *iter;
		std::string res = match[1].str();
		input = match[2].str();
		return res;
	}
	return "";
}
std::vector<Operand*>* Parser::getOperands(std::string operandList) {
	std::vector<Operand*> *res = new std::vector<Operand*>();
	try {
		while (operandList != "") {
			std::string opstr = getNextCSV(operandList);
			res->push_back(parseOperand(opstr));
			if (operandList != "") operandList.erase(0, 1);
		}
	}
	catch (OperandException& e) {
		Operand::freeOperandList(res);
		throw;
	}
	return res;

}
long long Parser::parseLiteral(std::string lit) {
	long long res;
	try {
		lit = Converter::toLower(Converter::removeBlanks(lit));
		if (std::regex_match(lit, HEX_LITPattern))
			res = std::stoll(lit, nullptr, 16);
		else res = std::stoll(lit);
	}
	catch (std::exception e) {
		throw OperandException("Parser::OperandException: Incorrect literal value!");
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
	if (reg[0] != 'r') throw OperandException("Parser::OperandException: Incorrect register operator!");
	std::string num = reg.substr(1);
	int res;
	try {
		res = std::stoi(num);
	}
	catch (std::exception e) {
		throw OperandException("Parser::OperandException: Incorrect register operand number!");
	}
	if (res < 0 || res > 15) throw OperandException("Parser::OperandException: Incorrect register operand number!");
	return res;
}
Operand* Parser::parseOperand(std::string operand) {
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
	return new SpecialOperand(operand);
	//throw OperandException("Parser::OperandException: Invalid operand expression: " + operand);
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
		std::string name = match[2].str();
		std::string operandList = match[3].str();
		std::vector<Operand*>* operands = getOperands(operandList);
		*dirp = new Directive(name, operands);
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
		std::string operandList = match[3];
		std::vector<Operand*>* operands = getOperands(operandList);
		*insp = new Instruction(mnemonic, operands);
		*dirp = nullptr;
		return;
	}

	throw ParserException(std::string("Parser::ParserException: Incorrect assembly syntax for line '") + line +"'.");
}
