#ifndef _SECTION_HEADER_TABLE_H_
#define _SECTION_HEADER_TABLE_H_
#include <string>
#include <list>
#include <regex>
#include "Exception.h"

class SectionHeaderTable {
public:
	class Entry {
	public:
		std::string name;
		char type;
		int offset;
		int size;
		Entry(std::string name, char type, int offset, int size) {
			this->name = name;
			this->type = type;
			this->offset = offset;
			this->size = size;
		}
		std::string str() {
			return name + "," + type + "," + std::to_string(offset) + "," + std::to_string(size);
		}
	};
private:
	std::vector<Entry>* entries;
	

public:
	SectionHeaderTable() {
		entries = new std::vector<Entry>();
	}
	void addEntry(std::string name, char type, int offset, int size) {
		entries->push_back(Entry(name, type, offset, size));
	}
	void addEntryFromFile(std::string line) {
		entries->push_back(getEntryFromFile(line));
	}
	std::string str() {
		std::string result = "SECTION HEADER TABLE\n";
		for (auto it = entries->begin(); it != entries->end(); it++) {
			result += it->str() + "\n";
		}
		return result;
	}
	Entry getEntryFromFile(std::string line) {
		std::regex pattern(R"del(^([\.\w]+),(\w),(\d+),(\d+)\s*$)del");
		std::sregex_iterator iter = std::sregex_iterator(line.begin(), line.end(), pattern);
		std::sregex_iterator end;
		if (iter == end) throw Exception("SectionHeaderTable: Invalid input file content (" + line + ")");
		std::smatch match = *iter;
		return Entry(match[1].str(), match[2].str().at(0), std::stoi(match[3].str()), std::stoi(match[4].str()));
	}
	int getCount() {
		return entries->size();
	}
	Entry getEntry(int i) {
		return entries->at(i);
	}
};

#endif