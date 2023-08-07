#ifndef _RELOCATIONTABLE_H_
#define _RELOCATIONTABLE_H_
#include <string>
#include <list>
#include "Exception.h"
#include "SymbolTable.h"

#define REL_PC12 "REL_PC12" //symbol - pc + addend (12 bits), 4 bits past the address
#define REL_32	"REL_32" //symbol + addend (32 bits)

/*
* Represents a relocation table. (duh)
*/
class RelocationTable {
public:
	struct Entry {
		int offset;
		std::string type;
		std::string symbol;
		int addend;
		Entry(int offset, std::string type, std::string symbol, int addend);
		std::string str();
	};
	class RelocationException : public Exception {
	public:
		RelocationException(std::string msg) : Exception(msg) {}
	};

private:
	std::list<Entry> *list;
	std::string name;

	/*
	* Applies a single relocation to the data string.
	*/
	void applyOne(Entry entry, std::string& data, std::string section, SymbolTable*, std::unordered_map<std::string, int>* localSegmentLocs);

public:
	static void checkDisplacementSize(int displacement);
	RelocationTable(std::string name);
	~RelocationTable();
	void addEntry(int offset, std::string type, std::string symbol, int addend);
	/*
	* Parses the line representing an entry in the relocation table and adds the entry.
	* The line should be in the same format as the one produced by the str() method.
	*/
	void addEntryFromFile(std::string line);
	std::list<Entry>* getEntries();
	/*
	* Returns a string representing the relocation table, one line per entry,
	* first line title, ends with a newline.
	*/
	std::string str();
	//Returns number of entries.
	std::string getName();
	int getCount();
	/*
	* Applies all of the relocations to the data passed as the first argument, using the symbol table passed as the second argument.
	* The baseAddr is the base address of the section, which is necessary for PC relative relocations.
	*/
	std::string apply(std::string data, std::string section, SymbolTable*, std::unordered_map<std::string, int>* localSegmentLocs);
	
	/*
	* This method is used for merging, and applies only the relocations which can be applied at the time of merging. These are:
	* REL_PC12 relocations where the place of the relocation, and the symbol are in the same section, and
	* REL_32 relocations where the symbol is *ABS*.
	* The applied relocations are removed from the list of entries, so that the table can then be merged.
	*/
	std::string applyPartially(std::string data, std::string section, SymbolTable*, std::unordered_map<std::string, int>* localSegmentLocs);
	/*
	* Merges the relocation table passed as the first argument.
	* baseAddr is the base address of the section that the relTable originally referenced,
	* and the offset of every entry is increased by this amount.
	* localSegmentLocs should contain the relevant locations of all of the sections referenced in the
	* relTable, and this will be used to adjust the addend of relocations that use a local (section) symbol.
	*/
	void merge(RelocationTable* relTable, int baseAddr, std::unordered_map<std::string, int>* localSegmentLocs);
};

#endif