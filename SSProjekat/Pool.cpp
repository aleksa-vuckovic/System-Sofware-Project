#include "Pool.h"
#include "Converter.h"

Pool::Pool() {
	std::unordered_map<std::string, int>* symMap = new std::unordered_map<std::string, int>();
	std::unordered_map<long, int>* litMap = new std::unordered_map<long, int>();
	size = base = 0;
}
void Pool::addLiteral(long lit) {
	if (litMap->find(lit) == litMap->end()) return;
	litMap->insert({ lit, size });
	data += Converter::toHex(lit, 8);
	size += 8;
}
void Pool::addSymbol(std::string sym) {
	if (symMap->find(sym) == symMap->end()) return;
	symMap->insert({ sym, size });
	data += "00000000";
	size += 8;
}
int Pool::getAddr(long lit) {
	return litMap->at(lit);
}
int Pool::getAddr(std::string sym) {
	return symMap->at(sym);
}
void Pool::setBase(int val) {
	base = val;
}
int Pool::getSize() {
	return size;
}
Pool::~Pool() {
	delete symMap;
	delete litMap;
}