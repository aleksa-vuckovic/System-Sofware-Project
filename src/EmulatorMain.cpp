#include <string>
#include "../inc/Exception.h"
#include "../inc/Emulator.h"
#include <fstream>

int main(int argc, const char* args[]) {
	try {
		std::string inputName;
		if (argc < 2) throw Exception("Input file name missing.");
		inputName = std::string(args[1]);

		std::ifstream input(inputName);
		if (!input.is_open()) throw Exception("Input file could not be opened.");

		Emulator emulator(&input);
		emulator.emulate();
	}
	catch (Exception e) {
		std::cout << e.getMsg() << std::endl;
		std::cout << "Emulator execution failed." << std::endl;
		return -1;
	}
	std::cout << "Emulator finished successfully." << std::endl;
	return 0;
}
