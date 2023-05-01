#include "SymbolTable.h"
#include <regex>
#include <string>
SymbolTable::SymbolTable() {
	table = new std::map<std::string, Entry*>();
	globals = new std::set<std::string>();
}
SymbolTable::~SymbolTable() {
	for (auto& elem : *table) {
		delete elem.second;
	}
	delete table; table = nullptr;
	delete globals; globals = nullptr;
}
void SymbolTable::addSymbol(unsigned val, char type, std::string section, std::string name) {
	if (table->find(name) != table->end()) throw SymbolException("SymbolTable::SymbolException: Double symbol definition.");
	char bind = 'L';
	if (globals->count(name)) globals->erase(name), bind = 'G';
	Entry* entry = new Entry(val, type, bind, section, name);
	table->insert({ name, entry });
}
bool SymbolTable::contains(std::string name) {
	return table->find(name) != table->end();
}
SymbolTable::Entry* SymbolTable::getEntry(std::string name) {
	return table->at(name); //throws exception if the key doesn't exist
}
void SymbolTable::setGlobal(std::string name) {
	if (table->find(name) != table->end()) {
		Entry* e = table->at(name);
		e->type = 'G';
	}
	else globals->insert(name);
}

void SymbolTable::addEntryFromFile(std::string line) {
	//value,type,bind,section,name
	std::regex pattern(R"del(^(\d+),(\w),(\w),(\w+),(\w+)$)del");
	std::sregex_iterator iter = std::sregex_iterator(line.begin(), line.end(), pattern);
	std::sregex_iterator end;
	if (iter == end) throw SymbolException("Incorrect format for symbol table entry.");
	std::smatch match = *iter;
	int val = stoi(match[1].str());
	char type = match[2].str()[0];
	char bind = match[3].str()[0];
	std::string section = match[4].str();
	std::string name = match[5].str();
	Entry* entry = new Entry(val, type, bind, section, name);
	table->insert({ name, entry });
}
std::string SymbolTable::str() {
	std::string res = "SYMTAB\n";
	for (auto& elem : *table) {
		res += elem.second->str() + "\n";
	}
	return res;
}

SymbolTable::Entry::Entry(unsigned val, char type, char bind, std::string section, std::string name) {
	this->value = val;
	this->type = type;
	this->bind = bind;
	this->section = section;
	this->name = name;
}

std::string SymbolTable::Entry::str()
{
	return std::to_string(value) + "," + type + "," + bind + "," + section + "," + name;
}
