#ifndef _POOL_H_
#define _POOL_H_
#include <unordered_map>
#include <string>
#include "RelocationTable.h"
class Pool {
	std::string data;
	int size, base;
	std::unordered_map<std::string, int>* symMap;
	std::unordered_map<long long, int>* litMap;
public:
	Pool();
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
	std::string getData();
	std::unordered_map<std::string, int>* getSymMap();
};

#endif