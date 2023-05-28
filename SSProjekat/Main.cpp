#include <iostream>
#include "Parser.h"
#include "Tests.h"
#include "Converter.h";

int main() {
	
	try {
		//testConverter();
		//testMemory();
		//testParser();
		//testAssembler();
		testLinker();
	}
	catch (Exception e) {
		std::cout << e.getMsg() << std::endl;
	}
	//std::cout << Converter::toLittleEndian("01234567");
}