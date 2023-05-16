#ifndef _SECTION_H_
#define _SECTION_H_
#include "RelocationTable.h"
#include "Pool.h"
#include <vector>
#include "Instruction.h"
#include "InstructionTranslator.h"
#include "Converter.h"
#include "SymbolTable.h"
#define MAX_PCREL_OFFSET 0x7FFFFF
#define MIN_PCREL_OFFSET -0x800000
class Section {
	InstructionTranslator* translator;
	Instruction* segmentEndInstruction;
	RelocationTable* relTable;
	std::string name;

	std::vector<Pool*>* pools;//pools, one after every code segment, although the last one may be empty
	std::vector<int>* codeAddr;//code segment addreses (pools are in between the segments)
	/*
	*	The addreses of the code segments and pools, as well as the contents of the pools, is determined during phase1
	*	Whenever a symbol or literal is encountered (except for jmp and branch instructions where pc relative addressing is used),
	*	if the previous pool exists, contains it and is within offset limits, no new entries are needed.
	*	Otherwise, if the next pool exists and contains it, no new entries are needed.
	*	Otherwise, an entry is added to the next pool (and if it doesn't exist yet, it is created).
	* 
	*	When the offset from the first instruction that references the next pool and the end of the next pool reaches the limit,
	*	which is checked before every new instruction/directive is added, a new segments is created, and positioned after that pool.
	*/
	//phase1 variables
	Pool* prev, * next;
	int curCodeSize;
	int curCodeStart;
	int switchPoint; //the point at which the next pool was first referenced (and created)
	int pos; //both phases
	//phase 2 variables
	int curSeg;
	std::string data;

	bool isWithinOffset(int length);
	void finishCodeSegment();
	void switchToNewPool();
	bool segmentEnd(int addSize);
public:
	Section(std::string name);
	~Section();
	void addInstruction(Instruction* ins);
	void addDirective(Directive* dir);
	void finishPhase1();

	void checkSegmentSwitch();

	void addInstruction2(Instruction* ins, SymbolTable* symTab);
	void addDirective2(Directive* dir, SymbolTable* symTab);
	void finishPhase2(SymbolTable* symTab);

	int getPos();
	std::string getName();

	std::string str(int max_line_length);
	int getRowCount(int max_line_length);
	RelocationTable* getRelTable();
};

#endif