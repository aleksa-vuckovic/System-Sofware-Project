#include <iostream>
#include "../inc/Parser.h"
#include "../inc/Tests.h"
#include "../inc/Converter.h";
#include "../inc/Exception.h"
#include <fstream>
#include "../inc/Assembler.h"
#include "../inc/Linker.h"
#include <regex>
#include "../inc/Emulator.h"

int main() {
	Tests tests;
	try {
		//tests.testConverter();
		//tests.testMemory();
		//tests.testParser();
		//tests.testEquDirective();
		//tests.testEmulatorTerminal();
		
		//tests.testSystem1();
		//tests.testSystem2();
		//tests.testSystem3();
		//tests.testSystem4();
		//tests.testSystem5();
		//tests.testSystem6();
		//tests.testSystem7();
		//tests.testSystem8();
		tests.testSystem9();
	}
	catch (Exception e) {
		std::cout << e.getMsg() << std::endl;
	}
}

