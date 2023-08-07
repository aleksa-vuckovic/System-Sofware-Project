#include <string>
#include "../inc/Exception.h"
#include <fstream>
#include "../inc/Assembler.h"

int main(int argc, const char* args[]) {
	try {
		std::string outputName = "";
		std::string inputName = "";
		for (int i = 1; i < argc; i++) {
			std::string arg = std::string(args[i]);
			if (arg == "-o" && i != argc - 1) outputName = std::string(args[++i]);
			else if (arg[0] == '-') throw Exception("Unknown command line option: '" + arg + "'");
			else inputName = arg;
		}
		if (inputName == "") throw Exception("Input file must be specified.");
		if (outputName == "") throw Exception("Output file name must be specified.");

		std::ifstream input(inputName);
		if (!input.is_open()) throw Exception("Failed to open file '" + inputName + "' for reading.");
		Assembler assembler;
		std::string result = assembler.assemble(&input);
		input.close();

		std::ofstream output(outputName);
		if (!output.is_open()) throw Exception("Failed to open file '" + outputName + "' for writing.");
		output << result;
		output.close();
	}
	catch (Exception e) {
		std::cout << e.getMsg() << std::endl;
		std::cout << "Assembler execution failed." << std::endl;
		return -1;
	}

	std::cout << "Assembler finished successfully." << std::endl;
	return 0;
}