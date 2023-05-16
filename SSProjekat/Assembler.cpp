#include "Assembler.h"
#include <string>
#include "Parser.h"
#include <map>
#include <regex>
#define USER_SECTION_LINE_LEN 40



std::string Assembler::assemble(std::istream& input) {
	std::map<std::string, Section*> sections;
	std::list<void*> lines;
	std::list<char> lineTypes;
	SymbolTable* symTable = new SymbolTable();
	SectionHeaderTable* shTable = new SectionHeaderTable();
	Section* curSection = nullptr;

	Parser parser;

	std::string line;
	std::string label = "";
	Directive* directive = nullptr;
	Instruction* instruction = nullptr;
	while (std::getline(input, line)) {
		std::regex pattern("^\\s*$");
		if (std::regex_match(line, pattern)) continue;
		parser.parseAssemblerLine(line, &label, &directive, &instruction);
		if (label != "") symTable->addSymbol(curSection->getPos(), 'O', curSection->getName(), label);
		if (directive) {
			if (directive->getName() == "section") {
				std::string name = directive->getOperand(0)->getSymbol();
				if (sections.count(name) > 0) curSection = sections.at(name);
				else {
					curSection = new Section(name);
					sections.insert({ name, curSection });
				}
			}
			curSection->addDirective(directive);
			lines.push_back(directive); lineTypes.push_back('D');


			if (directive->getName() == "global") {
				for (int i = 0; i < directive->getOperandCount(); i++) symTable->setGlobal(directive->getOperand(i)->getSymbol());
			}
			else if (directive->getName() == "extern") {
				for (int i = 0; i < directive->getOperandCount(); i++) {
					std::string name = directive->getOperand(i)->getSymbol();
					symTable->addSymbol(curSection->getPos(), 'U', "*UND*", name);
					symTable->setGlobal(name);
				}
			}
			else if (directive->getName() == "end") {
				break;
			}
		}
		if (instruction) {
			curSection->addInstruction(instruction);
			lines.push_back(instruction); lineTypes.push_back('I');
		}
		
		label = "";
		directive = nullptr;
		instruction = nullptr;
	}
	for (auto it = sections.begin(); it != sections.end(); it++) {
		Section* section = (*it).second;
		section->finishPhase1();
	}
	//

	curSection = nullptr;
	auto it = lines.begin();
	auto it2 = lineTypes.begin();
	while (it != lines.end()) {
		
		void* ptr = *it;
		if (*it2 == 'I') {
			Instruction* instruction = (Instruction*)ptr;
			curSection->addInstruction2(instruction, symTable);
		}
		else {
			Directive* directive = (Directive*)ptr;
			if (directive->getName() == "section") curSection = sections.at(directive->getOperand(0)->getSymbol());
			curSection->addDirective2(directive, symTable);
		}
		it++;
		it2++;
	}
	for (auto it = sections.begin(); it != sections.end(); it++) {
		Section* section = (*it).second;
		section->finishPhase2(symTable);
		symTable->addSymbol(0, 'S', section->getName(), section->getName());
	}

	//Making the section header table and the output string
	std::string resultStart = "FILE HEADER\n" + std::to_string(sections.size() * 2 + 1) + "\n";
	int startLine = 4 + sections.size() * 2; //the line where the symbol table begins
	shTable->addEntry("SYMTAB", 'S', startLine, symTable->getCount() + 1);
	std::string resultEnd = symTable->str();
	startLine += symTable->getCount() + 1;
	for (auto it = sections.begin(); it != sections.end(); it++) {
		Section* section = (*it).second;
		RelocationTable* relTable = section->getRelTable();
		shTable->addEntry(section->getName(), 'L', startLine, section->getRowCount(USER_SECTION_LINE_LEN) + 1);
		resultEnd += section->str(USER_SECTION_LINE_LEN);
		startLine += 1 + section->getRowCount(USER_SECTION_LINE_LEN);
		shTable->addEntry(".rela" + section->getName(), 'R', startLine, relTable->getCount() + 1);
		resultEnd += ".rela" + section->getName() + "\n" + relTable->str();
		startLine += 1 + relTable->getCount();
	}
	std::string result = resultStart + shTable->str() + resultEnd;
	return result;
}