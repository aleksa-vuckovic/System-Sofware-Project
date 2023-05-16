#include "Parser.h"
#include <iostream>
#include "Tests.h"
#include "Assembler.h"
#include <sstream>
#include "Exception.h"
#define TESTNUM 10
void testParser() {
	std::string lines[TESTNUM] = {
		" label1: beQ %pC, %r15, $-0xf ",
		" lab2:",
		"ld [%r0 + symbol1], %R4",
		"st %r9, [%r5 - 0xf]",
		"ld [%r4], %r4",
		"ld $0x100, %r4",
		"ld $immsym, %r3",
		".dir arg1,arg2",
		".dir -0xff, hello",
		"ld 0x01, %r0"
	};
	Instruction* ins = nullptr;
	Directive* dir = nullptr;
	std::string label = "";
	Parser parser;

	for (int i = 0; i < TESTNUM; i++) {
		std::cout << std::to_string(i) << ". ";
		try {
			parser.parseAssemblerLine(lines[i], &label, &dir, &ins);
			if (label != "") std::cout << label + ": ";
			if (ins) std::cout << ins->str();
			if (dir) std::cout << dir->str();
			std::cout << std::endl;
		}
		catch (Parser::ParserException& e) {
			std::cout << e.getMsg() << std::endl;
		}
		if (ins) delete ins;
		if (dir) delete dir;
		ins = nullptr; dir = nullptr; label = "";
	}
}


void testAssembler() {
	Assembler assembler;

	std::string program =
		".section .text\n"
		"push %r0\n"
		"ld 0x01, %r0\n"
		"st %r1, var\n"
		".section .data\n"
		"var: .word -1\n"
		".end";
	std::istringstream input = std::istringstream(program);
	try {
		std::string result = assembler.assemble(input);
		std::cout << result;
	}
	catch (Exception e) {
		std::cout << e.getMsg() << std::endl;
	}
	
}