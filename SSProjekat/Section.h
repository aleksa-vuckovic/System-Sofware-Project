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
#define USER_SECTION_LINE_LEN 40
/*
* This class is used during assembling, and produces the content and relocation table of a single loadable section.
*/
class Section {
	InstructionTranslator* translator;
	Instruction* segmentEndInstruction;
	RelocationTable* relTable;
	std::string name;

	std::vector<Pool*>* pools;//pools, one after every code segment, although the last one may be empty
	std::vector<int>* codeAddr;//code segment addreses (pools are in between the segments)
	/*
	*	The addreses of the code segments and pools, as well as the contents of the pools, is determined during phase 1.
	*	Whenever a symbol or literal is encountered (except for jmp and branch instructions where pc relative addressing is used),
	*	if the previous pool exists, contains it and is within offset limits, no new entries are needed.
	*	Otherwise, if the next pool exists and contains it, no new entries are needed.
	*	Otherwise, an entry is added to the next pool.
	*	If it doesn't exist yet, a new one is created, and the location where the pool was first referenced is remembered
	*	as switchPoint, for the purposes of checking the size of the offset from that location to the end of the pool,
	*	which must remain within offset limits (worst case scenario).
	* 
	*	When the offset from the first instruction that references the next pool to the end of the next pool reaches the limit,
	*	which is checked before every new instruction/directive is added, a new segment is created, and positioned after that pool.
	*	A jmp instruction is inserted at the end of the segment, so that the control flow continues past the literal pool.
	*/
	//phase 1 variables
	Pool* prev, * next;
	int curCodeSize;
	int curCodeStart;
	int switchPoint; //the point at which the next pool was first referenced (and created)
	int pos; //used in both phases
	//phase 2 variables
	int curSeg;
	std::string data;

	//Checks if the distance is within the offset limits for a 12 bit displacement.
	bool isWithinOffset(int length);
	//Called whenever the offset limit is reached and code is moved past the next pool.
	void finishCodeSegment();
	//Called when the first symbol or literal is encounterd that does not exist in the previous pool or exists and is not within the offset limit.
	void switchToNewPool();
	/*
	* Checks if the segment should end.The offset used is calculated as the size of the current code segment starting from the switchPoint, plus the addSize, which should be
	* the size of the next instruction/directive to be inserted, plus the size of the jmp instruction, plus the size of the pool.
	* Should be called before every instruction/directive is added, and if it returns true, finishCodeSegment() should be called.
	*/
	
	bool segmentEnd(int addSize);
	/*
	* Similar to segmentEnd in purpose, but this method is used in phase 2, and if the segment does end,
	* inserts the jmp instruction if necessary, adds the pool data to the final string, and updates the relevant fields.
	* Should be called after every instruction/directive is added.
	*/
	void checkSegmentSwitch();

public:
	Section(std::string name);
	~Section();
	void addInstruction(Instruction* ins);
	void addDirective(Directive* dir);
	//At the end of phase 1, the number of pools is always the same as the number of code sections (the last pool might be empty).
	void finishPhase1();


	void addInstruction2(Instruction* ins, SymbolTable* symTab);
	void addDirective2(Directive* dir, SymbolTable* symTab);
	void finishPhase2(SymbolTable* symTab);

	int getPos();
	std::string getName();
	std::string getData();

	/*
	* Returns the section content separted into lines according to the argument.
	* The first line is the name of the section.
	* Ends with newline.
	*/
	std::string str(int max_line_length);
	/*
	* Same as above, but for external data.
	*/
	static std::string str(std::string name, std::string data, int max_line_length);
	int getRowCount(int max_line_length);
	static int getRowCount(std::string data, int max_line_length);
	RelocationTable* getRelTable();
};

#endif