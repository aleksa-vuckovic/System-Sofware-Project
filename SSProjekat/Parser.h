#ifndef _PARSER_H_
#define _PARSER_H_
#include "Instruction.h"
#include <regex>
#include "Operand.h"
#include "Exception.h"
class Parser {
	std::regex directivePattern, instructionPattern, labelOnlyPattern;
	std::regex operatorListPattern;

	std::regex IMM_LITPattern, IMM_SYMPattern, MEM_LITPattern, MEM_SYMPattern;
	std::regex REG_DIRPattern, REG_INDPattern, REG_LITPattern, REG_SYMPattern;
public:
	class ParserException : public Exception {
	public:
		ParserException(std::string msg) : Exception(msg) {}
	};
	class OperatorException : public ParserException {
	public:
		OperatorException(std::string msg) : ParserException(msg) {}
	};
	Parser();
	void toLower(std::string& s);
	int parseLiteral(std::string lit);
	int parseRegister(std::string reg);
	Operand* parseOperator(std::string operand);
	void parseAssemblerLine(std::string line, std::string* label, Directive** dirp, Instruction** insp);
};
#endif