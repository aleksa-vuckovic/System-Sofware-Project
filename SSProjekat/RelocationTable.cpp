#include "RelocationTable.h"
#include <string>
#include <regex>


RelocationTable::Entry::Entry(int offset, std::string type, std::string symbol, int addend) {
	this->offset = offset;
	this->type = type;
	this->symbol = symbol;
	this->addend = addend;
};

std::string RelocationTable::Entry::str() {
	return std::to_string(offset) + "," + type + "," + symbol + "," + std::to_string(addend);
}
RelocationTable::RelocationTable() {
	list = new std::list<Entry>();
}
RelocationTable::~RelocationTable() {
	delete list; list = nullptr;
}
void RelocationTable::addEntry(int offset, std::string type, std::string symbol, int addend) {
	list->push_back({ offset, type, symbol, addend });
}
void RelocationTable::addEntryFromFile(std::string line) {
	std::regex pattern(R"del(^(\d+),(\w+),(\w+),(\w+))del");
	std::sregex_iterator iter = std::sregex_iterator(line.begin(), line.end(), pattern);
	std::sregex_iterator end;
	if (iter == end) throw RelocationException("RelocationTable::RelocationException: Invalid input file format.");
	std::smatch match = *iter;
	addEntry(std::stoi(match[1].str()), match[2].str(), match[3].str(), std::stoi(match[4].str()));
}
std::list<RelocationTable::Entry>* RelocationTable::getEntries() {
	return list;
}

std::string RelocationTable::str() {
	std::string res = "";
	for (auto it = list->begin(); it != list->end(); it++) {
		res += it->str();
		res += "\n";
	}
	return res;
}