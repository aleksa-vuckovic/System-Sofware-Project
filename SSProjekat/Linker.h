#ifndef _LINKER_H_
#define _LINKER_H_
#include <string>
#include <fstream>
#include <unordered_map>
#include "SymbolTable.h"
#include "Exception.h"
class Linker {
	class LinkerException : public Exception {
	public:
		LinkerException(std::string msg) : Exception(msg) {}
	};

	SymbolTable* readSymTable(std::ifstream& input);
	std::string formatData(std::string data, int baseAddr);

public:
	
	std::string link(std::ifstream inputs[], int cnt, std::unordered_map<std::string, int>* locs);
	std::string merge(std::ifstream inputs[], int cnt);
	
};

#endif