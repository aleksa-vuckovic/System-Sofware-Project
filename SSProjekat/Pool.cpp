#include "Pool.h"
#include "Converter.h"

Pool::Pool() {
	symMap = new std::unordered_map<std::string, int>();
	litMap = new std::unordered_map<long long, int>();
	size = base = 0;
}
void Pool::addLiteral(long long lit) {
	if (litMap->find(lit) != litMap->end()) return;
	litMap->insert({ lit, size });
	data += Converter::toHex(lit, 8);
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

std::string Pool::getData()
{
	return data;
}
std::unordered_map<std::string, int>* Pool::getSymMap() {
	return symMap;
}