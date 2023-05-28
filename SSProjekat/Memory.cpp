#include "Memory.h"
#include <iostream>
#define TERM_OUT 0xFFFFFF00u
#define TERM_IN 0xFFFFFF04u
#define TIME_CONFIG 0xFFFFFF10u
Memory::Memory() {
}
Memory::~Memory() {
	for (auto it = data.begin(); it != data.end(); it++) {
		delete it->second;
	}
}
unsigned char Memory::getByte(unsigned addr) {
	unsigned char ret;
	if (data.find(addr / segSize) == data.end()) ret = 0;
	else ret = data[addr / segSize][addr % segSize];
	//if (addr == 0x400000EC) std::cout << (int)ret << " read from 0x400000EC" << std::endl;
	return ret;
}
unsigned Memory::getInt(unsigned addr) {
	return ((unsigned)getByte(addr + 3) << 24) + ((unsigned)getByte(addr + 2) << 16) + ((unsigned)getByte(addr + 1) << 8) + ((unsigned)getByte(addr));
}
void Memory::writeByte(unsigned addr, unsigned char byte) {
	if (data.find(addr / segSize) == data.end()) {
		char* arr = new char[segSize];
		for (int i = 0; i < segSize; i++) arr[i] = 0;
		data[addr / segSize] = arr;
	}
	data[addr / segSize][addr % segSize] = byte;
	//if (addr == 0x400000EC) std::cout << (int)byte << " written to 0x400000EC" << std::endl;
}
void Memory::writeInt(unsigned addr, unsigned int num) {
	writeByte(addr, num % 256); num >>= 8;
	writeByte(addr + 1, num % 256); num >>= 8;
	writeByte(addr + 2, num % 256); num >>= 8;
	writeByte(addr + 3, num % 256);
	if (addr == TERM_OUT) termOutWritten = true;
}
bool Memory::termOutWasWritten() {
	bool ret = termOutWritten;
	termOutWritten = false;
	return ret;
}
int Memory::getTermOut() {
	return getInt(TERM_OUT);
}
void Memory::writeTermIn(char c) {
	writeByte(TERM_IN, c);
}
int Memory::getTimeConfig() {
	return getInt(TIME_CONFIG);
}