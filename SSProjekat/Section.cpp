#include "Section.h"
#include "SymbolTable.h"
#include "Exception.h"
#include <unordered_map>
#include <vector>
#include "Operand.h"
#include "Parser.h"
bool Section::isWithinOffset(int length) {
	return length <= (1 << 12) - 1 && length >= -(1 << 12);
}
void Section::finishCodeSegment() {
	pools->push_back(next);
	codeAddr->push_back(curCodeStart);
	next->setBase(curCodeStart + curCodeSize + translator->getSize(segmentEndInstruction));
	curCodeStart = pos = next->getBase() + next->getSize();
	curCodeSize = 0;
	prev = next;
	next = nullptr;
}
void Section::switchToNewPool() {
	next = new Pool();
	switchPoint = pos;
}
bool Section::segmentEnd(int addSize) {
	return next && !isWithinOffset(curCodeStart + curCodeSize - switchPoint + addSize + translator->getSize(segmentEndInstruction) + next->getSize() + 4);
}
Section::Section(std::string name) {
	std::vector<Operand*>* ops = new std::vector<Operand*>();
	ops->push_back(new MemoryLiteralOperand(0x0));
	translator = new InstructionTranslator();
	segmentEndInstruction = new Instruction("jmp", ops);
	relTable = new RelocationTable(".rela" + name);
	this->name = name;
	curCodeStart = curCodeSize = pos = 0;
	pools = new std::vector<Pool*>();
	codeAddr = new std::vector<int>();
	prev = next = nullptr;
	switchPoint = -1;
}
Section::~Section() {
	for (auto it = pools->begin(); it != pools->end(); it++) {
		delete* it;
	}
	delete pools; pools = nullptr;
	delete codeAddr; codeAddr = nullptr;
	delete segmentEndInstruction; segmentEndInstruction = nullptr;
	delete translator; translator = nullptr;
	delete relTable; relTable = nullptr;
}
void Section::addInstruction(Instruction* ins) {
	int size = translator->getSize(ins);
	//Check if code segment should end
	if (segmentEnd(size)) finishCodeSegment();
	curCodeSize += size;
	pos += size;
	std::string mne = ins->getMnemonic();

	if (!(mne == "call" || mne == "jmp" || mne == "beq" || mne == "bne" || mne == "bgt")) for (int i = 0; i < ins->getOperandCount(); i++) {
		//Jump instructions never require pools, as they use pc relative locations
		Operand* o = ins->getOperand(i);
		if (o->hasLiteral()) {
			long long lit = o->getLiteral();
			if (!(prev && prev->hasLiteral(lit) && isWithinOffset(curCodeSize + prev->getSize()))) {
				if (!next) switchToNewPool();
				next->addLiteral(lit);
			}
		}
		if (o->hasSymbol()) {
			std::string sym = o->getSymbol();
			if (!(prev && prev->hasSymbol(sym) && isWithinOffset(curCodeSize + prev->getSize()))) {
				if (!next) switchToNewPool();
				next->addSymbol(sym);
			}
		}
	}
}
void Section::addDirective(Directive* dir) {
	std::string name = dir->getName();
	int size = translator->getSize(dir);
	//Check if code segment should end
	if (segmentEnd(size)) finishCodeSegment();
	curCodeSize += size;
	pos += size;
}
void Section::finishPhase1() {
	if (!next) next = new Pool(); //Empty pool, so that the end of the code segment is known in the next phase
	pools->push_back(next);
	next->setBase(curCodeStart + curCodeSize);
	codeAddr->push_back(curCodeStart);
	pos = 0;
	curSeg = 0;
	data = "";
}

void Section::checkSegmentSwitch() {
	int end = pools->at(curSeg)->getBase();
	if (curSeg == codeAddr->size() - 1) {
		//Last segment, no jmp instruction before pool
		if (pos != end) return;
		data += pools->at(curSeg)->getData();
		pos += pools->at(curSeg)->getSize();
		curSeg++;
	}
	else {
		if (pos + translator->getSize(segmentEndInstruction) != end) return;
		//Insert jmp instruction to skip over the pool
		pos += translator->getSize(segmentEndInstruction);
		int litoffset;
		std::string jmp = translator->translate(segmentEndInstruction, &litoffset, nullptr);
		std::string val = Converter::toHex(pools->at(curSeg)->getSize(), 3);
		jmp.replace(litoffset, 3, val);
		data += jmp;

		data += pools->at(curSeg)->getData();
		pos += pools->at(curSeg)->getSize();
		curSeg++;
	}
}

void Section::addInstruction2(Instruction* ins, SymbolTable* symTab) {
	Pool* prev, * next;
	if (curSeg > 0) prev = pools->at(curSeg - 1);
	else prev = nullptr;
	next = pools->at(curSeg);
	std::string mne = ins->getMnemonic();
	int size = translator->getSize(ins);
	int litoffset, reloffset;
	std::string translation = translator->translate(ins, &litoffset, &reloffset);
	if (mne == "call" || mne == "jmp" || mne == "beq" || mne == "bne" || mne == "bgt") {
		//Generate a pc relative relocation
		Operand* keyOp;
		if (mne == "call" || mne == "jmp") keyOp = ins->getOperand(0);
		else keyOp = ins->getOperand(2);
		if (keyOp->getType() == Operand::MEM_LIT) {
			//Literal
			long long lit = keyOp->getLiteral();
			//Add a pcrel relocation with addend=literal-2
			relTable->addEntry(pos + reloffset, REL_PC12, "", lit - 2);
		}
		else {
			//symbol
			std::string sym = keyOp->getSymbol();
			//then add a pccrel relocation for the symbol
			if (!symTab->contains(sym)) throw Exception("Exception: Undefined symbol used in a directive.");
			SymbolTable::Entry* entry = symTab->getEntry(sym);
			if (entry->bind == 'L') relTable->addEntry(pos + reloffset, REL_PC12, entry->section, entry->value - 2);
			else relTable->addEntry(pos + reloffset, REL_PC12, sym, -2);
		}
	}
	else {
		for (int i = 0; i < ins->getOperandCount(); i++) {
			Operand* op = ins->getOperand(i);
			if (op->hasLiteral()) {
				long long lit = op->getLiteral();
				int location;
				if (prev && prev->hasLiteral(lit)) location = prev->getAddr(lit);
				else location = next->getAddr(lit);
				std::string val = Converter::toHex(location - pos - litoffset - 2, 3);
				translation.replace(litoffset*2 + 1, 3, val);
				break; //There can be only one literal, or one symbol
			}
			else if (op->hasSymbol()) {
				std::string sym = op->getSymbol();
				int location;
				if (prev && prev->hasSymbol(sym)) location = prev->getAddr(sym);
				else location = next->getAddr(sym);
				std::string val = Converter::toHex(location - pos - litoffset - 2, 3);
				translation.replace(litoffset*2 + 1, 3, val);
				break;
			}
		}
	}
	data += translation;
	pos += size;
	checkSegmentSwitch();
}
void Section::addDirective2(Directive* dir, SymbolTable* symTab) {
	std::string name = dir->getName();
	if (name == "word") {
		for (int i = 0; i < dir->getOperandCount(); i++) {
			Operand* op = dir->getOperand(i);
			if (op->getType() == Operand::MEM_LIT) data += Converter::toLittleEndian(Converter::toHex32(op->getLiteral()));
			else {
				//relocation necessary
				data += "00000000";
				std::string sym = op->getSymbol();
				if (!symTab->contains(sym)) throw Exception("Exception: Undefined symbol used in a directive.");
				SymbolTable::Entry* entry = symTab->getEntry(sym);
				if (entry->bind == 'L') relTable->addEntry(pos, REL_32, entry->section, entry->value);
				else relTable->addEntry(pos, REL_32, sym, 0);
			}
			pos += 4;
		}
	}
	else if (name == "skip") {
		int toSkip = dir->getOperand(0)->getLiteral();
		for (int i = 0; i < toSkip; i++) data += "00";
		pos += toSkip;
	}
	else if (name == "ascii") {
		Parser parser;
		std::string chars = parser.parseAsciiString(dir->getOperand(0)->getOriginalString());
		for (char c : chars) data += Converter::toHex(c, 2);
		data += "00";
	}
	else return;
	checkSegmentSwitch();
}
void Section::finishPhase2(SymbolTable* symTab) {
	//add relocations for symbols in all of the pools
	for (int i = 0; i < pools->size(); i++) pools->at(i)->addRelocations(symTab, relTable);
}

std::string Section::str(int max_line_length) {
	return str(name, data, max_line_length);
}
std::string Section::str(std::string name, std::string data, int max_line_length) {
	std::string res = name + "\n";
	for (int i = 0; i < data.length(); i += max_line_length) {
		int len = i + max_line_length > data.length() ? data.length() - i : max_line_length;
		res += data.substr(i, len) + "\n";
	}
	return res;
}
int Section::getRowCount(int max_line_length) {
	return getRowCount(data, max_line_length);
}
int Section::getRowCount(std::string data, int max_line_length) {
	return (data.length() - 1 + max_line_length) / max_line_length;
}
RelocationTable* Section::getRelTable() {
	return relTable;
}

int Section::getPos() {
	return pos;
}
std::string Section::getName() {
	return name;
}

std::string Section::getData()
{
	return data;
}
