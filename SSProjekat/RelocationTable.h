#ifndef _RELOCATIONTABLE_H_
#define _RELOCATIONTABLE_H_
#include <string>
#include <list>
#include "Exception.h"
#include "SymbolTable.h"

#define REL_PC12 "REL_PC12" //symbol - pc + addend (12 bits), 4 bits past the address
#define REL_32	"REL_32" //symbol + addend (32 bits)

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

public:
	RelocationTable();
	~RelocationTable();
	void addEntry(int offset, std::string type, std::string symbol, int addend);
	void addEntryFromFile(std::string line);
	std::list<Entry>* getEntries();
	std::string str();
	int getCount();
	std::string apply(std::string, int baseAddr, SymbolTable*);
	void merge(RelocationTable* relTable, int baseAddr, std::unordered_map<std::string, int>* localSegmentLocs);
};

#endif