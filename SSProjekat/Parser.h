#ifndef _PARSER_H_
#define _PARSER_H_
#include "Instruction.h"
#include <regex>
#include "Operand.h"
#include "Exception.h"
#include <vector>
class Parser {
	std::regex directivePattern, instructionPattern, labelOnlyPattern;
	//std::regex operatorListPattern;
	std::regex commaSeparatorPattern;

	std::regex IMM_LITPattern, IMM_SYMPattern, MEM_LITPattern, MEM_SYMPattern;
	std::regex REG_DIRPattern, REG_INDPattern, REG_LITPattern, REG_SYMPattern;
	std::regex HEX_LITPattern;
	std::string getNextCSV(std::string& input);
	std::vector<Operand*>* getOperands(std::string operandList);
public:
	class ParserException : public Exception {
	public:
		ParserException(std::string msg) : Exception(msg) {}
	};
	class OperandException : public ParserException {
	public:
		OperandException(std::string msg) : ParserException(msg) {}
	};
	Parser();

	long long parseLiteral(std::string lit);
	int parseRegister(std::string reg);
	Operand* parseOperand(std::string operand);
	void parseAssemblerLine(std::string line, std::string* label, Directive** dirp, Instruction** insp);
};
#endif