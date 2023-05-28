#ifndef _TESTS_H_
#define _TESTS_H_
#include <unordered_map>

class Tests {
	
	std::string testSystem(std::string files[], int cnt, std::unordered_map<std::string, int>* locs);
public:
	void testConverter();
	void testMemory();
	void testParser();


	void testSystem1();
	void testSystem2();
	void testSystem3();
};
#endif