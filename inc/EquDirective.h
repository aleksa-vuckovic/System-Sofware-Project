#ifndef _EQUDIRECTIVE_H_
#define _EQUDIRECTIVE_H_
#include <string>
#include <vector>
#include <regex>
#include "SymbolTable.h"
#include "Instruction.h"

class EquDirective {
	std::string symbol;
	std::vector<std::string> addSyms;
	std::vector<std::string> subSyms;
	long long literal;

	/*
	* Returns the next blank separated segment of the expression.
	* That should be either a '+', a '-', a literal, or a symbol.
	* The returned segment is removed from expr.
	*/
	std::string getNextPart(std::string& expr);
	
	/*
	* Converts to a literal if possible, if not throws exception.
	*/
	long long getLiteral(std::string op);
public:
	EquDirective(Directive* dir);
	bool canBeResolved(SymbolTable* symTab);
	void resolve(SymbolTable* symTab);

	std::string str();

};


#endif