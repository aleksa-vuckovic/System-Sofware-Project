#ifndef _POOL_H_
#define _POOL_H_
#include <unordered_map>
#include <string>
#include "RelocationTable.h"

/*
* Represents a pool of literals and symbol addreses used in a code section.
* The pool saves the location of each literal/symbol relative to the start of the pool.
* The base address of the pool can also be set.
* The addRelocations() method adds absolute relocations for all of the symbols contained in the pool.
*/
class Pool {
	std::string data;
	int size, base;
	std::unordered_map<std::string, int>* symMap;
	std::unordered_map<long long, int>* litMap;
	std::string section;
public:
	Pool(std::string section);
	~Pool();
	void addLiteral(long long lit);
	void addSymbol(std::string sym);
	bool hasLiteral(long long lit);
	bool hasSymbol(std::string sym);
	int getAddr(long long lit);
	int getAddr(std::string sym);
	void setBase(int val);
	int getBase();
	int getSize();
	void addRelocations(SymbolTable* symTab, RelocationTable* relTable);
	std::string getData();
};

#endif