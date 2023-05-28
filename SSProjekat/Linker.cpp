#include "Linker.h"
#include "SectionHeaderTable.h"
#include <map>
#include "RelocationTable.h"
#include <string>
#include <fstream>
#include "Converter.h"
#include "Section.h"
#include "Assembler.h"


SymbolTable* Linker::readSymTable(std::ifstream& is) {
	is.seekg(0);
	std::string line;
	std::getline(is, line);
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

void Linker::readSections(std::ifstream& input, std::unordered_map<std::string, std::string>* data, std::unordered_map<std::string, RelocationTable*>* rels) {
	std::string line;

	//Skipping the title.
	std::getline(input, line);
	//Reading the number of entries in the SHT.
	std::getline(input, line);
	int sectionCnt = std::stoi(line);
	//Skipping the title.
	std::getline(input, line);
	SectionHeaderTable sht;
	//Reading the SHT.
	for (int j = 0; j < sectionCnt; j++) {
		std::getline(input, line);
		sht.addEntryFromFile(line);
	}
	//Reading all of the sections.
	for (int j = 0; j < sht.getCount(); j++) {
		//Assuming that the order of entries in the SHT is the same as the order in the file.
		SectionHeaderTable::Entry entry = sht.getEntry(j);
		std::getline(input, line);
		int lines = entry.size - 1;
		if (entry.type == 'L') {
			std::string result = "";
			for (int k = 0; k < lines; k++) {
				std::getline(input, line);
				result += line;
				if (result.back() == '\n') result.erase(result.length() - 1);
			}
			data->insert({ entry.name, result });
		}
		else if (entry.type == 'R') {
			RelocationTable* res = new RelocationTable(entry.name);
			for (int k = 0; k < lines; k++) {
				std::getline(input, line);
				res->addEntryFromFile(line);
			}
			rels->insert({ entry.name.substr(5), res });
		}
		else if (entry.type == 'S') {
			//Skipping the symbol table
			for (int k = 0; k < lines; k++) std::getline(input, line);
		}
	}
}

void Linker::readSections(std::ifstream inputs[], int cnt, std::vector<std::unordered_map<std::string, std::string>*>* data, std::vector<std::unordered_map<std::string, RelocationTable*>*>* rels, std::unordered_map<std::string, int>* finalSizes) {
	for (int i = 0; i < cnt; i++) {
		std::unordered_map<std::string, std::string>* fileData = new std::unordered_map<std::string, std::string>();
		data->push_back(fileData);
		std::unordered_map<std::string, RelocationTable*>* fileRels = new std::unordered_map<std::string, RelocationTable*>();
		rels->push_back(fileRels);

		readSections(inputs[i], fileData, fileRels);

		if (!finalSizes) continue;
		for (auto it = fileData->begin(); it != fileData->end(); it++) {
			if (finalSizes->count(it->first) == 0) {
				finalSizes->insert({ it->first, 0 });
			}
			(*finalSizes)[it->first] += it->second.length() / 2;
		}
	}
}

std::string Linker::formatData(std::string data, int baseAddr) {
	std::string result = "";
	int len = data.length();
	int cur = 0;
	while (cur < len / 2) {
		int start = baseAddr + cur;
		int skip = start % 8;
		start = start - skip;
		int end = start + 8 - baseAddr;
		int excess = 0;
		if (end > len / 2) {
			excess = end - len / 2;
			end = len / 2;
		}
		result += Converter::toHex(start, 8) + ":";
		for (int i = 0; i < skip; i++) result = result + "   ";
		for (int i = cur; i < end; i++) result += " " + data.substr(i * 2, 2);
		for (int i = 0; i < excess; i++) result += "   ";
		result += "\n";
		cur = end;
	}
	return result;
}

void Linker::calculateSegmentLocations(std::vector<std::unordered_map<std::string, std::string>*>* localData, std::unordered_map<std::string, int>* baseLocations, std::vector<std::unordered_map<std::string, int>*>* outputLocations) {
	std::unordered_map<std::string, int> curLocs;
	for (auto it = baseLocations->begin(); it != baseLocations->end(); it++) curLocs.insert({ it->first, it->second });
	for (int i = 0; i < localData->size(); i++) {
		std::unordered_map<std::string, std::string>* data = localData->at(i);
		std::unordered_map<std::string, int>* localSegmentLocs = new std::unordered_map<std::string, int>();
		outputLocations->push_back(localSegmentLocs);
		for (auto it = data->begin(); it != data->end(); it++) {
			localSegmentLocs->insert({ it->first, curLocs.at(it->first) });
			curLocs[it->first] += it->second.length() / 2;
		}
	}
}

SymbolTable* Linker::getMergedSymbolTable(std::ifstream inputs[], int cnt, std::vector<std::unordered_map<std::string, int>*>* localSegmentLocations) {
	SymbolTable* symTab = new SymbolTable();
	for (int i = 0; i < cnt; i++) {
		SymbolTable* t = readSymTable(inputs[i]);
		symTab->mergeEntries(t, localSegmentLocations->at(i));
		delete t;
	}
	return symTab;
}

std::string Linker::link(std::ifstream inputs[], int cnt, std::unordered_map<std::string, int>* locs) {
	//The final starting locations of every section.
	std::unordered_map<std::string, int> finalLocs;
	//The final sizes of every section.
	std::unordered_map<std::string, int> finalSizes;
	//The final data of every section.
	std::unordered_map<std::string, std::string> finalData;
	//An array of maps, where each map coresponds to one file and contains its segment locations in the final layout.
	std::vector<std::unordered_map<std::string, int>*> localSegmentLocations;
	//An array of maps, where each map coresponds to one file and contains its data for every segment.
	std::vector<std::unordered_map<std::string, std::string>*> localData;
	//An array of maps, where each map coresponds to one file and contains its relocation tables for every segment (the keys are segment names, without .rela)
	std::vector<std::unordered_map<std::string, RelocationTable*>*> localRelTables;
	
	readSections(inputs, cnt, &localData, &localRelTables, &finalSizes);
	
	/*
	* Calculating the location of each output section.
	* Explicitly specified locations are inserted first, and the rest after.
	*/
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

	calculateSegmentLocations(&localData, &finalLocs, &localSegmentLocations);

	//The symbol table is created by merging all of the individual tables.
	SymbolTable* symTab = getMergedSymbolTable(inputs, cnt, &localSegmentLocations);
	symTab->checkNoUndef();
	for (auto it = finalLocs.begin(); it != finalLocs.end(); it++) symTab->addSymbol(it->second, 'S', it->first, it->first);

	//The relocations are applied and the final section content is created
	for (int i = 0; i < cnt; i++) {
		std::unordered_map<std::string, std::string>* data = localData.at(i);
		std::unordered_map<std::string, RelocationTable*>* rels = localRelTables.at(i);
		std::unordered_map<std::string, int>* localSegmentLocs = localSegmentLocations.at(i);
		for (auto it = data->begin(); it != data->end(); it++) {
			std::string data = it->second;
			RelocationTable* rel = rels->at(it->first);
			data = rel->apply(data, localSegmentLocs->at(it->first), symTab);
			if (finalData.find(it->first) == finalData.end()) finalData.insert({ it->first, data });
			else finalData[it->first] += data;
		}
	}

	//Finally, the sections are sorted accoring to their location and the final result is formed, while checking that there is no overlap
	std::string result = "";

	int cnt2 = finalLocs.size();
	int* positions = new int[cnt2];
	std::string* names = new std::string[cnt2];
	int i = 0;
	for (auto it = finalLocs.begin(); it != finalLocs.end(); it++) {
		positions[i] = it->second;
		names[i++] = it->first;
	}
	for (int i = 1; i < cnt2; i++) {
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
	for (int i = 0; i < cnt2 - 1; i++) {
		if (positions[i] + finalSizes.at(names[i]) > positions[i + 1]) throw LinkerException("Linker::LinkerException: Overlapping segments.");
	}
	for (int i = 0; i < cnt2; i++) {
		result += formatData(finalData.at(names[i]), finalLocs.at(names[i]));
	}
	//cleanup
	delete[] positions;
	delete[] names;
	delete symTab;
	for (auto it = localSegmentLocations.begin(); it != localSegmentLocations.end(); it++) delete* it;
	for (auto it = localData.begin(); it != localData.end(); it++) delete* it;
	for (auto it = localRelTables.begin(); it != localRelTables.end(); it++) {
		for (auto it2 = (*it)->begin(); it2 != (*it)->end(); it2++) delete it2->second;
		delete* it;
	}
	return result;

}

std::string Linker::merge(std::ifstream inputs[], int cnt) {
	std::unordered_map<std::string, RelocationTable*> finalRelTables;
	std::unordered_map<std::string, std::string> finalData;
	std::vector<std::unordered_map<std::string, int>*> localSegmentLocations;
	std::vector<std::unordered_map<std::string, std::string>*> localData;
	std::vector<std::unordered_map<std::string, RelocationTable*>*> localRelTables;
	

	readSections(inputs, cnt, &localData, &localRelTables, nullptr);

	//Making the final data
	for (int i = 0; i < cnt; i++) {
		std::unordered_map<std::string, std::string>* data = localData.at(i);
		for (auto it = data->begin(); it != data->end(); it++) {
			if (finalData.count(it->first) == 0) finalData.insert({ it->first, "" });
			finalData[it->first] += it->second;
		}
	}

	std::unordered_map<std::string, int> baseLocs; for (auto it = finalData.begin(); it != finalData.end(); it++) baseLocs.insert({ it->first, 0 });
	calculateSegmentLocations(&localData, &baseLocs, &localSegmentLocations);


	SymbolTable* symTab = getMergedSymbolTable(inputs, cnt, &localSegmentLocations);
	for (auto it = finalData.begin(); it != finalData.end(); it++) symTab->addSymbol(0, 'S', it->first, it->first);

	//The relocation tables are created by merging the relocation tables of the corresponding sections.
	for (int i = 0; i < cnt; i++) {
		std::unordered_map<std::string, std::string>* data = localData.at(i);
		std::unordered_map<std::string, RelocationTable* >* rels = localRelTables.at(i);
		std::unordered_map<std::string, int>* locs = localSegmentLocations.at(i);
		for (auto it = data->begin(); it != data->end(); it++) {
			if (finalRelTables.find(it->first) == finalRelTables.end()) finalRelTables.insert({ it->first, new RelocationTable(".rela" + it->first)});
			finalRelTables.at(it->first)->merge(rels->at(it->first), locs->at(it->first), locs);
		}
	}
	
	Assembler assembler;
	std::string result = assembler.getRelocatableFile(&finalData, &finalRelTables, symTab);

	//Cleanup
	for (auto it = localRelTables.begin(); it != localRelTables.end(); it++) {
		for (auto it2 = (*it)->begin(); it2 != (*it)->end(); it2++) delete it2->second;
		delete* it;
	}
	localRelTables.clear();
	for (auto it = localData.begin(); it != localData.end(); it++) delete* it;
	localData.clear();
	for (auto it = localSegmentLocations.begin(); it != localSegmentLocations.end(); it++) delete* it;
	localSegmentLocations.clear();
	finalData.clear();
	for (auto it = finalRelTables.begin(); it != finalRelTables.end(); it++) delete it->second;
	finalRelTables.clear();
	delete symTab; symTab = nullptr;

	return result;
}