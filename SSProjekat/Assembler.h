#ifndef _ASSEMBLER_H_
#define _ASSEMBLER_H_
#include <iostream>
#include "Section.h"
#include "SymbolTable.h"
#include "SectionHeaderTable.h"

class Assembler {

public:
	std::string assemble(std::istream& input);

};

#endif