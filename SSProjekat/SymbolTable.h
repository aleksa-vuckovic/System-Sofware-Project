#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_
#include <string>
#include <map>
#include <set>
#include <unordered_map>
#include "Exception.h"
class SymbolTable {
public:
	struct Entry {
		unsigned value;
		char type; //S - section, O - object, U - undefined
		char bind; //G - global, L - local
		std::string section; //
		std::string name; //
		Entry(unsigned val, char type, char bind, std::string section, std::string name);
		std::string str();
	};
	class SymbolException : public Exception {
	public:
		SymbolException(std::string message) : Exception(message) {}
	};
private:
	std::map<std::string, Entry*> *table;
	std::set<std::string> *globals;
public:
	SymbolTable();
	~SymbolTable();
	void addSymbol(unsigned val, char type, std::string section, std::string name);
	bool contains(std::string name);
	Entry* getEntry(std::string name);
	void setGlobal(std::string name);

	void addEntryFromFile(std::string line);
	std::string str();
	int getCount();
	void mergeEntries(SymbolTable* symTab, std::unordered_map<std::string, int>* sectionLocs);
	bool isDefined(std::string name);
	bool isUndefined(std::string name);
	void remove(std::string name);
	void checkNoUndef();
};

#endif