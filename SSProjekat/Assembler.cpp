#include "Assembler.h"
#include <string>
#include "Parser.h"
#include <map>
#include <regex>
#include "Exception.h"
#include <fstream>
#include "EquDirective.h"
#include <vector>



std::string Assembler::assemble(std::ifstream* input) {
	std::unordered_map<std::string, Section*> sections;
	//The parsed objects are remembered, so that the parser doesn't have to be used again
	std::list<void*> lines;
	std::list<char> lineTypes;
	SymbolTable* symTable = new SymbolTable();
	Section* curSection = nullptr;
	InstructionTranslator translator;
	std::vector<EquDirective*> equs;

	Parser parser;

	std::string line;
	std::string label = "";
	Directive* directive = nullptr;
	Instruction* instruction = nullptr;
	//PHASE 1
	while (std::getline(*input, line)) {
		line = parser.removeComment(line);
		if (parser.emptyLine(line)) continue; //Empty lines are allowed and ignored.
		parser.parseAssemblerLine(line, &label, &directive, &instruction);
		/*
		* For a label to be valid, it has to come after a .section directive.
		* The special case "label: .section" is not considered here.
		*/
		if (label != "") {
			if (!curSection) throw AssemblerException("Label definition must come after a .section directive.");
			symTable->addSymbol(curSection->getPos(), 'O', curSection->getName(), label);
		}
		if (directive) {
			translator.checkDirective(directive);
			if (directive->getName() == "section") {
				std::string name = directive->getOperand(0)->getSymbol();
				if (sections.count(name) > 0) curSection = sections.at(name);
				else {
					curSection = new Section(name);
					sections.insert({ name, curSection });
				}
			}
			if (curSection) curSection->addDirective(directive);
			lines.push_back(directive); lineTypes.push_back('D');


			if (directive->getName() == "global") {
				for (int i = 0; i < directive->getOperandCount(); i++) symTable->setGlobal(directive->getOperand(i)->getSymbol());
			}
			else if (directive->getName() == "extern") {
				for (int i = 0; i < directive->getOperandCount(); i++) {
					std::string name = directive->getOperand(i)->getSymbol();
					symTable->addSymbol(0, 'U', "*UND*", name);
					symTable->setGlobal(name);
				}
			}
			else if (directive->getName() == "end") {
				break;
			}
			else if (directive->getName() == "equ") {
				equs.push_back(new EquDirective(directive));
			}
		}
		if (instruction) {
			if (!curSection) throw AssemblerException("Instructions must come after a .section directive.");
			translator.checkInstruction(instruction);
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
	//Resolving equ directives
	bool progress = true;
	while (equs.size() > 0 && progress) {
		progress = false;
		for (int i = 0; i < equs.size(); i++) {
			EquDirective* cur = equs.at(i);
			if (cur->canBeResolved(symTable)) {
				cur->resolve(symTable);
				progress = true;
				equs.erase(equs.begin() + i);
				i--;
				delete cur;
			}
		}
	}
	if (equs.size() != 0) throw AssemblerException("Equ directive is not resolvable.");
	//END OF PHASE 1

	curSection = nullptr;
	auto it = lines.begin();
	auto it2 = lineTypes.begin();
	//PHASE 2
	while (it != lines.end()) {
		
		void* ptr = *it;
		if (*it2 == 'I') {
			Instruction* instruction = (Instruction*)ptr;
			curSection->addInstruction2(instruction, symTable);
			delete instruction;
		}
		else {
			Directive* directive = (Directive*)ptr;
			if (directive->getName() == "section") curSection = sections.at(directive->getOperand(0)->getSymbol());
			if (curSection) curSection->addDirective2(directive, symTable);
			delete directive;
		}
		it++;
		it2++;
	}
	lines.clear(); lineTypes.clear();
	for (auto it = sections.begin(); it != sections.end(); it++) {
		Section* section = (*it).second;
		section->finishPhase2(symTable);
		symTable->addSymbol(0, 'S', section->getName(), section->getName());
	}
	//END OF PHASE 2
	std::string result = getRelocatableFile(&sections, symTable);

	//Cleanup
	delete symTable; symTable = nullptr;
	for (auto it = sections.begin(); it != sections.end(); it++) delete it->second;
	sections.clear();

	return result;

}

std::string Assembler::getRelocatableFile(std::unordered_map<std::string, Section*>* sections, SymbolTable* symTable) {
	std::unordered_map<std::string, std::string> data;
	std::unordered_map<std::string, RelocationTable*> relTables;
	for (auto it = sections->begin(); it != sections->end(); it++) {
		Section* sec = it->second;
		data.insert({ it->first, sec->getData() });
		relTables.insert({ it->first, sec->getRelTable() });
	}
	return getRelocatableFile(&data, &relTables, symTable);
}

std::string Assembler::getRelocatableFile(std::unordered_map<std::string, std::string>* sections, std::unordered_map<std::string, RelocationTable*>* relTables, SymbolTable* symTab) {
	SectionHeaderTable shTable;
	std::string resultStart = "FILE HEADER\n" + std::to_string(sections->size() * 2 + 1) + "\n";
	int startLine = 4 + sections->size() * 2; //the line where the symbol table begins
	shTable.addEntry("SYMTAB", 'S', startLine, symTab->getCount() + 1);
	std::string resultEnd = symTab->str();
	startLine += symTab->getCount() + 1;
	auto it1 = sections->begin();
	auto it2 = relTables->begin();
	for (; it1 != sections->end(); it1++, it2++) {
		std::string name = it1->first;
		std::string data = it1->second;
		RelocationTable* relTable = it2->second;
		shTable.addEntry(name, 'L', startLine, Section::getRowCount(data, USER_SECTION_LINE_LEN) + 1);
		resultEnd += Section::str(it1->first, data, USER_SECTION_LINE_LEN);
		startLine += 1 + Section::getRowCount(data, USER_SECTION_LINE_LEN);
		shTable.addEntry(relTable->getName(), 'R', startLine, relTable->getCount() + 1);
		resultEnd += relTable->str();
		startLine += 1 + relTable->getCount();
	}
	std::string result = resultStart + shTable.str() + resultEnd;
	return result;
}