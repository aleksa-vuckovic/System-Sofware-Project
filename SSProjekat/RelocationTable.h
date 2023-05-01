#ifndef _RELOCATIONTABLE_H_
#define _RELOCATIONTABLE_H_
#include <string>
#include <list>
#include "Exception.h"

#define REL_PC12 "REL_PC12"
#define REL_32	"REL_32"

class RelocationTable {
public:
	struct Entry {
		int offset;
		std::string type;
		std::string symbol;
		int addend;
		Entry(int offset, std::string type, std::string symbol, int addend);
		std::string str();
	};
	class RelocationException : public Exception {
	public:
		RelocationException(std::string msg) : Exception(msg) {}
	};

private:
	std::list<Entry> *list;

public:
	RelocationTable();
	~RelocationTable();
	void addEntry(int offset, std::string type, std::string symbol, int addend);
	void addEntryFromFile(std::string line);
	std::list<Entry>* getEntries();
	std::string str();
};

#endif