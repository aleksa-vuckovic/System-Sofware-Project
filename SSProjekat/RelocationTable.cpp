#include "RelocationTable.h"
#include <string>
#include <regex>
#include "Converter.h"
#define MAX_PC12 ((1 << 11) - 1)
#define MIN_PC12 (-(1 << 11))												


RelocationTable::Entry::Entry(int offset, std::string type, std::string symbol, int addend) {
	this->offset = offset;
	this->type = type;
	this->symbol = symbol;
	this->addend = addend;
};

std::string RelocationTable::Entry::str() {
	return std::to_string(offset) + "," + type + "," + symbol + "," + std::to_string(addend);
}
RelocationTable::RelocationTable(std::string name) {
	list = new std::list<Entry>();
	this->name = name;
}
RelocationTable::~RelocationTable() {
	delete list; list = nullptr;
}
void RelocationTable::addEntry(int offset, std::string type, std::string symbol, int addend) {
	list->push_back({ offset, type, symbol, addend });
}
void RelocationTable::addEntryFromFile(std::string line) {
	std::regex pattern(R"del(^(\d+),(\w+),([\.\w]+),([+-]?\d+))del");
	std::sregex_iterator iter = std::sregex_iterator(line.begin(), line.end(), pattern);
	std::sregex_iterator end;
	if (iter == end) throw RelocationException("RelocationTable::RelocationException: Invalid input file format (" + line + ")");
	std::smatch match = *iter;
	addEntry(std::stoi(match[1].str()), match[2].str(), match[3].str(), std::stoi(match[4].str()));
}
std::list<RelocationTable::Entry>* RelocationTable::getEntries() {
	return list;
}

std::string RelocationTable::str() {
	std::string res = name + "\n";
	for (auto it = list->begin(); it != list->end(); it++) {
		res += it->str();
		res += "\n";
	}
	return res;
}
std::string RelocationTable::getName()
{
	return name;
}
int RelocationTable::getCount() {
	return list->size();
}
std::string RelocationTable::apply(std::string data, int baseAddr, SymbolTable* symTab, std::unordered_map<std::string, int>* localSegmentLocs) {
	for (auto it = list->begin(); it != list->end(); it++) {
		if (it->type == REL_PC12) {
			int pc = baseAddr + it->offset;
			int val = it->addend - pc;
			if (it->symbol != "") {
				if (localSegmentLocs->find(it->symbol) == localSegmentLocs->end()) val += symTab->getEntry(it->symbol)->value;
				else val += localSegmentLocs->at(it->symbol);
			}
			int start = it->offset * 2 + 1;
			data.replace(start, 3, Converter::toHex(val, 3));
		}
		else if (it->type == REL_32) {
			int val = it->addend;
			if (it->symbol != "") {
				if (localSegmentLocs->find(it->symbol) == localSegmentLocs->end()) val += symTab->getEntry(it->symbol)->value;
				else val += localSegmentLocs->at(it->symbol);
			}
			int start = it->offset * 2;
			data.replace(start, 8, Converter::toLittleEndian(Converter::toHex32(val)));
		}
	}
	return data;
}
void RelocationTable::merge(RelocationTable* relTable, int baseAddr, std::unordered_map<std::string, int>* localSegmentLocs) {
	for (auto it = relTable->list->begin(); it != relTable->list->end(); it++) {
		Entry entry(*it);
		if (localSegmentLocs->find(entry.symbol) != localSegmentLocs->end()) {
			//It's a section symbol, meaning that addend should be adjusted according to the new position of the section
			entry.addend += localSegmentLocs->at(entry.symbol);
		}
		entry.offset += baseAddr;
		list->push_back(entry);
	}
}