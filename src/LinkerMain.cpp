#include <string>
#include <regex>
#include <list>
#include "../inc/Parser.h"
#include <unordered_map>
#include "../inc/Linker.h"
#include <iostream>

int main(int argc, const char* args[]) {
	std::regex pattern(R"del(^-place=([\.\w]+)@([\w\d]+)$)del");
	std::sregex_iterator end;
	Parser parser;
	try {
		std::string outputName = "";
		std::list<std::string> inputNames;
		std::string type = "";
		std::unordered_map<std::string, int> locs;
		Linker linker;

		for (int i = 1; i < argc; i++) {
			std::string arg = std::string(args[i]);
			if (arg == "-o" && i != argc - 1) outputName = std::string(args[++i]);
			else if (arg == "-hex") type = "hex";
			else if (arg == "-relocatable") type = "rel";
			else if (std::regex_search(arg, pattern)) {
				std::sregex_iterator iter(arg.begin(), arg.end(), pattern);
				if (iter != end) {
					std::smatch match = *iter;
					std::string sectionName = match[1].str();
					int address = parser.parseLiteral(match[2].str());
					locs.insert({ sectionName, address });
				}
			}
			else if (arg[0] == '-') throw Exception("Unrecognized command line option: '" + arg + "'");
			else inputNames.push_back(arg);
		}
		if (type == "") throw Exception("One of the following two options must be specified: -hex or -relocatable.");
		if (outputName == "") throw Exception("Output file name must be specified.");
		if (inputNames.size() == 0) throw Exception("At least one input file name must be specified.");

		std::ifstream** inputs = new std::ifstream * [inputNames.size()];
		int i = 0;
		for (auto it = inputNames.begin(); it != inputNames.end(); it++) {
			inputs[i] = new std::ifstream(*it);
			if (!inputs[i]->is_open()) {
				for (int j = 0; j < i; j++) {
					inputs[j]->close(); delete inputs[j];
				}
				delete inputs[i];
				throw Exception("Input file '" + *it + "' could not be opened.");
			}
			i++;
		}

		std::string result;
		try {
			if (type == "hex") result = linker.link(inputs, inputNames.size(), &locs);
			else result = linker.merge(inputs, inputNames.size());
		}
		catch (Exception e) {
			for (int i = 0; i < inputNames.size(); i++) {
				inputs[i]->close(); delete inputs[i];
			}
			throw;
		}
		for (int i = 0; i < inputNames.size(); i++) {
			inputs[i]->close(); delete inputs[i];
		}

		std::ofstream output(outputName);
		if (!output.is_open()) throw Exception("Unable to open output file '" + outputName + "'");

		output << result;
	}
	catch (Exception e) {
		std::cout << e.getMsg() << std::endl;
		std::cout << "Linker execution failed." << std::endl;
		return -1;
	}
	std::cout << "Linker finished successfully." << std::endl;
	return 0;
}
