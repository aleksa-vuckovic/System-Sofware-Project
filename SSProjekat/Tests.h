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


	void testSystem1();
	void testSystem2();
	void testSystem3();
	void testSystem4();
	void testSystem5();
};
#endif