#include <iostream>
#include "Parser.h"
#include "Tests.h"
#include "Converter.h";

int main() {
	Tests tests;
	try {
		//tests.testConverter();
		//tests.testMemory();
		//tests.testParser();
		
		//tests.testSystem1();
		//tests.testSystem2();
		//tests.testSystem3();
	}
	catch (Exception e) {
		std::cout << e.getMsg() << std::endl;
	}
}