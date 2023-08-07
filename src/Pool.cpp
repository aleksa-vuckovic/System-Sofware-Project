#include "../inc/Pool.h"
#include "../inc/Converter.h"

Pool::Pool(std::string section) {
	symMap = new std::unordered_map<std::string, int>();
	litMap = new std::unordered_map<long long, int>();
	size = base = 0;
	this->section = section;
}
void Pool::addLiteral(long long lit) {
	if (litMap->find(lit) != litMap->end()) return;
	litMap->insert({ lit, size });
	data += Converter::toLittleEndian(Converter::toHex(lit, 8));
	size += 4;
}
void Pool::addSymbol(std::string sym) {
	if (symMap->find(sym) != symMap->end()) return;
	symMap->insert({ sym, size });
	data += "00000000";
	size += 4;
}
bool Pool::hasLiteral(long long lit) {
	return litMap->find(lit) != litMap->end();
}
bool Pool::hasSymbol(std::string sym) {
	return symMap->find(sym) != symMap->end();
}
int Pool::getAddr(long long lit) {
	return base + litMap->at(lit);
}
int Pool::getAddr(std::string sym) {
	return base + symMap->at(sym);
}
void Pool::setBase(int val) {
	base = val;
}
int Pool::getBase() {
	return base;
}
int Pool::getSize() {
	return size;
}
Pool::~Pool() {
	delete symMap; symMap = nullptr;
	delete litMap; litMap = nullptr;
}
void Pool::addRelocations(SymbolTable* symTab, RelocationTable* relTable) {
	for (const auto& pair : *symMap) {
		std::string sym = pair.first;
		int pos = pair.second;
		if (!symTab->contains(sym)) throw Exception("Exception: Undefined symbol " + sym + " used in an instruction.");
		SymbolTable::Entry* entry = symTab->getEntry(sym);
		if (entry->section == "*ABS*") data.replace(pos * 2, 8, Converter::toLittleEndian(Converter::toHex32(entry->value)));
		else if (entry->bind == 'L') relTable->addEntry(base + pos, REL_32, entry->section, entry->value);
		else relTable->addEntry(base + pos, REL_32, sym, 0);
	}
}
std::string Pool::getData()
{
	return data;
}