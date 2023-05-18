#include "Linker.h"
#include "SectionHeaderTable.h"
#include <map>
#include "RelocationTable.h"
#include <string>
#include <fstream>
#include "Converter.h"
#include "Section.h"


SymbolTable* Linker::readSymTable(std::ifstream& is) {
	std::string line;
	std::getline(is, line); //
	std::getline(is, line);
	int sectionCount = std::stoi(line);
	std::getline(is, line);
	SectionHeaderTable sht;
	int curline = 3;
	for (int i = 0; i < sectionCount; i++) {
		std::getline(is, line);
		curline++;
		SectionHeaderTable::Entry entry = sht.getEntryFromFile(line);
		if (entry.name == "SYMTAB") {
			int start = entry.offset + 1;
			int cnt = entry.size - 1;
			while (curline != start) curline++, std::getline(is, line);
			SymbolTable* ret = new SymbolTable();
			for (int i = 0; i < cnt; i++) {
				std::getline(is, line);
				ret->addEntryFromFile(line);
			}
			return ret;
		}
	}
	return nullptr;
}

std::string Linker::link(std::ifstream inputs[], int cnt, std::unordered_map<std::string, int>* locs) {
	std::unordered_map<std::string, int> finalLocs;
	std::unordered_map<std::string, int> finalSizes;//
	std::unordered_map<std::string, std::string> finalData;
	std::vector<std::unordered_map<std::string, int>*> localSegmentLocations;
	std::vector<std::unordered_map<std::string, std::string>*> localData;//
	std::vector<std::unordered_map<std::string, RelocationTable*>*> relTables;//
	//Retrieving all of the user sections and relocation sections from every input file,
	//while also calculating the sizes of the final sections
	for (int i = 0; i < cnt; i++) {
		std::unordered_map<std::string, std::string>* data = new std::unordered_map<std::string, std::string>();
		localData.push_back(data);
		std::unordered_map<std::string, RelocationTable*>* rels = new std::unordered_map<std::string, RelocationTable*>();
		relTables.push_back(rels);
		std::string line;

		std::getline(inputs[i], line);
		std::getline(inputs[i], line);
		int sectionCnt = std::stoi(line);
		std::getline(inputs[i], line);
		SectionHeaderTable* sht = new SectionHeaderTable();
		for (int j = 0; j < sectionCnt; j++) {
			std::getline(inputs[i], line);
			sht->addEntryFromFile(line);
		}
		for (int j = 0; j < sht->getCount(); j++) {
			//assuming that the order of entries in the SHT is the same as the order in the file
			SectionHeaderTable::Entry entry = sht->getEntry(j);
			std::getline(inputs[i], line);
			int lines = entry.size;
			if (entry.type == 'L') {
				std::string result = "";
				for (int k = 0; k < lines; k++) {
					std::getline(inputs[i], line);
					result += line;
					if (result.back() == '\n') result.erase(result.length() - 1);
				}
				data->insert({ entry.name, result });
				if (finalSizes.count(entry.name) == 0) {
					finalSizes.insert({ entry.name, 0 });
					finalData.insert({ entry.name, "" });
				}
				finalSizes.insert({ entry.name, finalSizes.at(entry.name) + result.length() });
				finalData.insert({ entry.name, finalData.at(entry.name) + result });
			}
			else if (entry.type == 'R') {
				RelocationTable* res = new RelocationTable;
				for (int k = 0; k < lines; k++) {
					std::getline(inputs[i], line);
					res->addEntryFromFile(line);
				}
				rels->insert({ entry.name.substr(4), res });
			}
		}

	}
	//Calculating the location of each output section
	//Explicitly specified locations are inserted first, and the rest after
	int maxLoc = 0;
	for (auto it = locs->begin(); it != locs->end(); it++) {
		finalLocs.insert({ it->first, it->second });
		if (it->second > maxLoc) maxLoc = it->second + finalSizes.at(it->first);
	}
	for (auto it = finalSizes.begin(); it != finalSizes.end(); it++) {
		if (finalLocs.find(it->first) == finalLocs.end()) {
			finalLocs.insert({ it->first, maxLoc });
			maxLoc += finalSizes.at(it->first);
		}
	}
	//The symbol table is created by merging all of the individual tables
	std::unordered_map<std::string, int> curLocs;
	for (auto it = finalLocs.begin(); it != finalLocs.end(); it++) {
		curLocs.insert({ it->first, it->second });
	}
	SymbolTable* symTab = new SymbolTable();
	for (int i = 0; i < cnt; i++) {
		std::unordered_map<std::string, std::string>* data = localData.at(i);
		std::unordered_map<std::string, RelocationTable* >* rels = relTables.at(i);
		std::unordered_map<std::string, int>* localSegmentLocs = new std::unordered_map<std::string, int>();
		localSegmentLocations.push_back(localSegmentLocs);
		for (auto it = data->begin(); it != data->end(); it++) {
			localSegmentLocs->insert({ it->first, curLocs.at(it->first)});
			curLocs.insert({ it->first, curLocs.at(it->first) + it->second.length() });
		}
		inputs[i].seekg(0);
		SymbolTable* t = readSymTable(inputs[i]);
		symTab->mergeEntries(t, localSegmentLocs);
	}
	symTab->checkNoUndef();
	//The relocations are applied and the final section content is created
	for (int i = 0; i < cnt; i++) {
		std::unordered_map<std::string, std::string>* data = localData.at(i);
		std::unordered_map<std::string, RelocationTable*>* rels = relTables.at(i);
		for (auto it = data->begin(); it != data->end(); it++) {
			std::string data = it->second;
			RelocationTable* rel = rels->at(it->first);
			std::unordered_map<std::string, int>* localSegmentLocs = localSegmentLocations.at(i);
			data = rel->apply(data, localSegmentLocs->at(it->first), symTab);
			if (finalData.find(it->first) == finalData.end()) finalData.insert({ it->first, data });
			else finalData.insert({ it->first, finalData.at(it->first) + data });
		}
	}

	//Finally, the sections are sorted accoring to their location and the final result is formed, while checking that there is no overlap
	std::string result = "";

	int cnt = finalLocs.size();
	int* positions = new int[cnt];
	std::string* names = new std::string[cnt];
	int i = 0;
	for (auto it = finalLocs.begin(); it != finalLocs.end(); it++) {
		positions[i] = it->second;
		names[i++] = it->first;
	}
	for (int i = 1; i < cnt; i++) {
		int j = i - 1;
		int pos = positions[i];
		std::string name = names[i];
		while (j >= 0 && positions[j] > pos) {
			positions[j + 1] = positions[j];
			names[j + 1] = names[j];
			j--;
		}
		positions[j + 1] = pos;
		names[j + 1] = name;
	}
	for (int i = 0; i < cnt - 1; i++) {
		if (positions[i] + finalSizes.at(names[i]) > positions[i + 1]) throw LinkerException("Linker::LinkerException: Overlapping segments.");
	}
	for (int i = 0; i < cnt; i++) {
		result += formatData(finalData.at(names[i]), finalLocs.at(names[i]));
	}
	//cleanup
	delete[] positions;
	delete[] names;
	delete symTab;
	for (auto it = localSegmentLocations.begin(); it != localSegmentLocations.end(); it++) delete* it;
	for (auto it = localData.begin(); it != localData.end(); it++) delete* it;
	for (auto it = relTables.begin(); it != relTables.end(); it++) delete* it;
	return result;

}

std::string Linker::formatData(std::string data, int baseAddr) {
	std::string result = "";
	int len = data.length();
	int cur = 0;
	while (cur < len) {
		int end = baseAddr + cur + 16;
		end -= end % 16;
		end -= baseAddr;
		if (end > len) end = len;
		int skip = (baseAddr + cur) % 16;
		result += Converter::toHex(baseAddr + cur, 4) + ":";
		for (int i = 0; i < skip; i++) result += "   ";
		for (int i = cur; i < end; i += 2) result += " " + data.substr(i, 2);
		data += "\n";
		cur = end;
	}
}

std::string Linker::merge(std::ifstream inputs[], int cnt) {
	std::unordered_map<std::string, RelocationTable*> finalRelTables;
	std::unordered_map<std::string, int> finalSizes;//
	std::unordered_map<std::string, std::string> finalData;
	std::vector<std::unordered_map<std::string, int>*> localSegmentLocations;
	std::vector<std::unordered_map<std::string, std::string>*> localData;//
	std::vector<std::unordered_map<std::string, RelocationTable*>*> relTables;//
	//Retrieving all of the user sections and relocation sections from every input file,
	//while also calculating the sizes of the final sections
	for (int i = 0; i < cnt; i++) {
		std::unordered_map<std::string, std::string>* data = new std::unordered_map<std::string, std::string>();
		localData.push_back(data);
		std::unordered_map<std::string, RelocationTable*>* rels = new std::unordered_map<std::string, RelocationTable*>();
		relTables.push_back(rels);
		std::string line;

		std::getline(inputs[i], line);
		std::getline(inputs[i], line);
		int sectionCnt = std::stoi(line);
		std::getline(inputs[i], line);
		SectionHeaderTable* sht = new SectionHeaderTable();
		for (int j = 0; j < sectionCnt; j++) {
			std::getline(inputs[i], line);
			sht->addEntryFromFile(line);
		}
		for (int j = 0; j < sht->getCount(); j++) {
			//assuming that the order of entries in the SHT is the same as the order in the file
			SectionHeaderTable::Entry entry = sht->getEntry(j);
			std::getline(inputs[i], line);
			int lines = entry.size;
			if (entry.type == 'L') {
				std::string result = "";
				for (int k = 0; k < lines; k++) {
					std::getline(inputs[i], line);
					result += line;
					if (result.back() == '\n') result.erase(result.length() - 1);
				}
				data->insert({ entry.name, result });
				if (finalSizes.count(entry.name) == 0) {
					finalSizes.insert({ entry.name, 0 });
					finalData.insert({ entry.name, "" });
				}
				finalSizes.insert({ entry.name, finalSizes.at(entry.name) + result.length() });
				finalData.insert({ entry.name, finalData.at(entry.name) + result });
			}
			else if (entry.type == 'R') {
				RelocationTable* res = new RelocationTable;
				for (int k = 0; k < lines; k++) {
					std::getline(inputs[i], line);
					res->addEntryFromFile(line);
				}
				rels->insert({ entry.name.substr(4), res });
			}
		}

	}
	//The symbol table is created by merging all of the individual tables
	//The relocation tables are created by merging the relocation tables of the corresponding sections
	std::unordered_map<std::string, int> curLocs;
	for (auto it = finalSizes.begin(); it != finalSizes.end(); it++) {
		curLocs.insert({ it->first, 0 });
	}
	SymbolTable* symTab = new SymbolTable();
	for (int i = 0; i < cnt; i++) {
		std::unordered_map<std::string, std::string>* data = localData.at(i);
		std::unordered_map<std::string, RelocationTable* >* rels = relTables.at(i);
		std::unordered_map<std::string, int>* localSegmentLocs = new std::unordered_map<std::string, int>();
		localSegmentLocations.push_back(localSegmentLocs);
		for (auto it = data->begin(); it != data->end(); it++) {
			localSegmentLocs->insert({ it->first, curLocs.at(it->first) });
			curLocs.insert({ it->first, curLocs.at(it->first) + it->second.length() });
		}
		for (auto it = data->begin(); it != data->end(); it++) {
			if (finalRelTables.find(it->first) == finalRelTables.end()) finalRelTables.insert({ it->first, new RelocationTable() });
			finalRelTables.at(it->first)->merge(rels->at(it->first), localSegmentLocs->at(it->first), localSegmentLocs);
		}
		inputs[i].seekg(0);
		SymbolTable* t = readSymTable(inputs[i]);
		symTab->mergeEntries(t, localSegmentLocs);
	}
	//
	//Making the section header table and the output string
	SectionHeaderTable shTable;
	std::string resultStart = "FILE HEADER\n" + std::to_string(finalData.size() * 2 + 1) + "\n";
	int startLine = 4 + finalData.size() * 2; //the line where the symbol table begins
	shTable.addEntry("SYMTAB", 'S', startLine, symTab->getCount() + 1);
	std::string resultEnd = symTab->str();
	startLine += symTab->getCount() + 1;
	auto it1 = finalData.begin();
	auto it2 = finalRelTables.begin();
	for (; it1 != finalData.end(); it1++, it2++) {
		std::string name = it1->first;
		std::string data = it1->second;
		RelocationTable* relTable = it2->second;
		shTable.addEntry(name, 'L', startLine, Section::getRowCount(data, USER_SECTION_LINE_LEN) + 1);
		resultEnd += Section::str(it1->first, data, USER_SECTION_LINE_LEN);
		startLine += 1 + Section::getRowCount(data, USER_SECTION_LINE_LEN);
		shTable.addEntry(".rela" + name, 'R', startLine, relTable->getCount() + 1);
		resultEnd += ".rela" + name + "\n" + relTable->str();
		startLine += 1 + relTable->getCount();
	}
	std::string result = resultStart + shTable.str() + resultEnd;
	return result;
}