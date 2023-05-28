#ifndef _LINKER_H_
#define _LINKER_H_
#include <string>
#include <fstream>
#include <unordered_map>
#include "SymbolTable.h"
#include "Exception.h"
#include "RelocationTable.h"
/*
* This class represents ~The Linker~.
*/
class Linker {
	class LinkerException : public Exception {
	public:
		LinkerException(std::string msg) : Exception(msg) {}
	};

	/*
	* Reads the symbol table from an input file, which should be in the same format as the assembler output.
	*/
	SymbolTable* readSymTable(std::ifstream& input);
	/*
	* Returns a string where each line is in the format "hexAddress: byte byte byte byte byte byte byte byte".
	* For example "0x10000000: 00 01 02 03 04 05 06 07".
	* The input string should be just hexadecimal digits.
	*/
	std::string formatData(std::string data, int baseAddr);

	/*
	* Reads loadable sections and relocation tables from input file and places them
	* inside the maps pointed to by the second and third arguments
	*/
	void readSections(std::ifstream& input, std::unordered_map<std::string, std::string>* data, std::unordered_map<std::string, RelocationTable*>* rels);
	/*
	* Reads loadable sections and relocation tables from all of the input files,
	* and places them inside a vector of maps, where each map coresponds to one input file.
	* The total size of each section, based on name, is also calculated placed inside the
	* map passed in as the last argument.
	*/
	void readSections(std::ifstream inputs[], int cnt, std::vector<std::unordered_map<std::string, std::string>*>* data, std::vector<std::unordered_map<std::string, RelocationTable*>*>* rels, std::unordered_map<std::string, int>* finalSizes);
	/*
	* Calculates the segment locations of each input file, represented by map of section name -> data,
	* based on the base locations of the final output sections, and writes them to the coresponding map of the outputLocations vector.
	*/
	void calculateSegmentLocations(std::vector<std::unordered_map<std::string, std::string>*>* localData, std::unordered_map<std::string, int>* baseLocations, std::vector<std::unordered_map<std::string, int>*>* outputLocations);
	/*
	* Merges the input symbol tables of each file into one symbol table.
	* The localSegmentLocations argument should contain the coresponding vector of maps,
	* which map the segment name to its location in the final output file.
	*/
	SymbolTable* getMergedSymbolTable(std::ifstream inputs[], int cnt, std::vector<std::unordered_map<std::string, int>*>* localSegmentLocations);
public:
	
	/*
	* Links the input files, and returns the executable file, int the format described above.
	* The locs map should contain the start addresses of sections, as sepcified in the command line.
	*/
	std::string link(std::ifstream inputs[], int cnt, std::unordered_map<std::string, int>* locs);
	/*
	* Merges the input files, into one output file with the same format.
	*/
	std::string merge(std::ifstream inputs[], int cnt);
};

#endif