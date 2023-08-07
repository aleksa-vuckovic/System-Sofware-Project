#ifndef _PARSER_H_
#define _PARSER_H_
#include "Instruction.h"
#include <regex>
#include "Operand.h"
#include "Exception.h"
#include <vector>

/*
* Used to parse the input assembly file, line by line,
* extracting the labels, instructions, directives and their operands.
* The parsing is done using regular expressions only.
*/
class Parser {
	//Assembly line patterns
	std::regex directivePattern, instructionPattern, labelOnlyPattern;
	//This patterns is used for comma separated values (operands), in combination with the getNextCSV() method
	std::regex commaSeparatorPattern;
	std::string getNextCSV(std::string& input);
	//Parses the operandList string and returns a coresponding vector of Operand objects
	std::vector<Operand*>* getOperands(std::string operandList);

	//Operand patterns
	std::regex IMM_LITPattern, IMM_SYMPattern, MEM_LITPattern, MEM_SYMPattern;
	std::regex REG_DIRPattern, REG_INDPattern, REG_LITPattern, REG_SYMPattern;
	std::regex HEX_LITPattern;

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

	/*
	* Removes single line comment from line, if it exists.
	* Single line comments start with #.
	*/
	std::string removeComment(std::string line);
	/*
	* Returns true if line contains only blank characters.
	*/
	bool emptyLine(std::string line);
	/*
	* Turns escape sequences like \n, \t, and \\ into single characters and returns
	* the modified string.
	*/
	std::string unescape(std::string data);
	/*
	* Extract the string in between the double quotes from the operand.
	*/
	std::string parseAsciiString(std::string operand);
	
	//Parses a literal from the string, removing all blanks and checking if the literal is hexadecimal.
	long long parseLiteral(std::string lit);
	//Parser a register name and returns a corresponding integer. Valid names are for example: status, pc, sp, r0, r1...
	int parseRegister(std::string reg);
	Operand* parseOperand(std::string operand);
	/*
	* Parses the assembler line, and writes the results into locations pointed to by the argument pointers.
	* If no label is present, *label gets value "". If no directive or no instruction is present, nullptr is written
	* into the corresponding location. Throws OperandException or ParserException in case of incorrect input.
	*/
	void parseAssemblerLine(std::string line, std::string* label, Directive** dirp, Instruction** insp);
};
#endif