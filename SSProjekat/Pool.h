#ifndef _POOL_H_
#define _POOL_H_
#include <unordered_map>
#include <string>
class Pool {
	std::string data;
	int size, base;
	std::unordered_map<std::string, int>* symMap;
	std::unordered_map<long, int>* litMap;

	Pool();
	~Pool();
	void addLiteral(long lit);
	void addSymbol(std::string sym);
	int getAddr(long lit);
	int getAddr(std::string sym);
	void setBase(int val);
	int getSize();


};

#endif