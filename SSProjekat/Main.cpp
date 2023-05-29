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
		//tests.testEquDirective();
		
		//tests.testSystem1();
		//tests.testSystem2();
		//tests.testSystem3();
		//tests.testSystem4();
		tests.testSystem5();
	}
	catch (Exception e) {
		std::cout << e.getMsg() << std::endl;
	}
}