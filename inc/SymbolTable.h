#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_
#include <string>
#include <map>
#include <set>
#include <unordered_map>
#include "Exception.h"

/*
* Represents a symbol table, with entries that contain the value, bind, section and name of a symbol.
*/
class SymbolTable {
public:
	struct Entry {
		unsigned value;
		char type; //S - section, O - object, U - undefined
		char bind; //G - global, L - local
		std::string section;
		std::string name;
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
	/*
	* If the symbol is in the table, sets binding to global.
	* Otherwise, the symbol is remembered as global, and when it's added,
	* The binding is automatically set to global.
	*/
	void setGlobal(std::string name);

	/*
	* Parses a line representing a symbol table entry, in the same format as
	* the one produced by the str() method of this class.
	*/
	void addEntryFromFile(std::string line);
	/*
	* Returns a string where each line is one entry,
	* with the first title line, ends with newline.
	*/
	std::string str();
	//Returns the number of entries.
	int getCount();
	/*
	* Adds all of the entries from symTab to this table, updating the values
	* according to the sectionLocs map (the value of the coresponding section in
	* the sectionLocs map is added to the original value from the symTab table).
	* Local entries are skiped.
	* Undefined entries are added only if they don't already exist in the table.
	*/
	void mergeEntries(SymbolTable* symTab, std::unordered_map<std::string, int>* sectionLocs);
	//Checks if the symbol exists and is defined.
	bool isDefined(std::string name);
	//Checks if the symbol exists and is undefined.
	bool isUndefined(std::string name);
	void remove(std::string name);
	//Throws exxception if undefined symbols exist in the table.
	void checkNoUndef();
};

#endif