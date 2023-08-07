#ifndef _TESTS_H_
#define _TESTS_H_
#include <unordered_map>

class Tests {
	
	void testSystem(std::string files[], int cnt, std::unordered_map<std::string, int>* locs, std::string correctEmulatorOutput);
public:
	void testConverter();
	void testMemory();
	void testParser();
	void testEquDirective();
	void testEmulatorTerminal();

	//Tests basic instruction, basic linking, prints one character
	void testSystem1();
	//Loops and more printing. Should print a sorted array of characters.
	void testSystem2();
	//Tests the intr instruction, and status register I flag. should print "inside".
	void testSystem3();
	//Tests the .ascii directive
	void testSystem4();
	//Tests the .equ directive. Should print "helo helo"
	void testSystem5();
	//Tests the timer interrupts and timer configuration.
	void testSystem6();
	//Tests some special cases related to absolute symbols.
	void testSystem7();
	/*
	* Tests character input. Reads a line from stdin, of up to 98 characters, end prints it back
	* when line feed is reached, 3 times.
	*/
	void testSystem8();
	/*
	* Tests using merged object files as input to linker. The program is the one from test 8.
	*/
	void testSystem9();
};
#endif