#ifndef _ASSEMBLER_H_
#define _ASSEMBLER_H_
#include <iostream>
#include "Section.h"
#include "SymbolTable.h"
#include "SectionHeaderTable.h"



/*
* This class represents ~The Assembler~.
*/
class Assembler {
public:
	class AssemblerException: public Exception {
	public:
		AssemblerException(std::string msg) : Exception("Assembler::AssemblerException: " + msg) {}
	};

	std::string assemble(std::ifstream* input);
	std::string getRelocatableFile(std::unordered_map<std::string, Section*>* sections, SymbolTable* symTable);
	std::string getRelocatableFile(std::unordered_map<std::string, std::string>* sections, std::unordered_map<std::string, RelocationTable*>* relTables, SymbolTable* symTab);
};

#endif