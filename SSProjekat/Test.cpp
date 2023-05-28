#include "Parser.h"
#include <iostream>
#include "Tests.h"
#include "Assembler.h"
#include <sstream>
#include "Exception.h"
#include "Linker.h"
#include "Emulator.h"
#include "Memory.h"

void Tests::testConverter() {
	std::cout << "TESTING CONVERTER" << std::endl << std::endl;
	std::string result = "";
	//Test 1
	long long num = 16256987;
	std::string val = Converter::toLower(Converter::toHex(num, 8));
	result += "Test 1: " + val + " | " + Converter::toLittleEndian(val) + "\n";
	//Test 2
	num = -200;
	result += "Test 2: " + Converter::toLower(Converter::toHex32(num)) + "\n";
	//Test 3
	result += "Test 3: " + Converter::toLower(Converter::removeBlanks("Text with \t Blank\n  Characters.")) + "\n";
	
	std::string correct = "Test 1: 00f80fdb | db0ff800\nTest 2: ffffff38\nTest 3: textwithblankcharacters.\n";
	std::cout << "Result:\n" << result << std::endl << "Correct:\n" << correct << std::endl << "Test passed: " << (result == correct ? "Yes" : "No") << std::endl;
}
void Tests::testMemory() {
	std::cout << "TESTING MEMORY" << std::endl << std::endl;
	std::string result = "";
	bool pass = true;
	int tmp;
	Memory mem;
	int num[] = { 1, 256, -190, 0xffffffff };
	int addr[] = { 0, 0xffffffff, 0xffffff00, 0xffffff10 };
	for (int i = 0; i < 4; i++) mem.writeInt(addr[i], num[i]);
	for (int i = 0; i < 4; i++) {
		if ((tmp = mem.getInt(addr[i])) != num[i]) {
			pass = false;
			std::cout << "Incorrect reading from address " << addr[i] << ": expected " << num[i] << ", got " << tmp << std::endl;
		}
	}
	if (mem.getTimeConfig() != num[3]) {
		std::cout << "Incorrect reading from Time Configuration Register" << std::endl;
		pass = false;
	}
	if (!mem.termOutWasWritten()) {
		std::cout << "Incorrect Terminal Out Register Flag" << std::endl;
		pass = false;
	}
	if (mem.getTermOut() != num[2]) {
		std::cout << "Incorrect reading from Terminal Out Register" << std::endl;
		pass = false;
	}

	mem.writeTermIn('a');
	if (mem.getInt(0xFFFFFF04) != (unsigned)'a') {
		std::cout << "Incorrect reading from Terminal In Register" << std::endl;
		pass = false;
	}
	std::cout << std::endl << "Test passed: " << (pass ? "Yes" : "No") << std::endl;

}
void Tests::testParser() {
	std::cout << "TESTING PARSER" << std::endl << std::endl;

	std::string result = "";
	std::string lines[] = {
		" label1: beQ %pC, %r15, $-0xf ",
		" lab2:",
		"ld [%r0 + symbol1], %R4",
		"st %r9, [%r5 - 0xf]",
		"ld [%r4], %r4",
		"ld $0x100, %r4",
		"ld $immsym, %r3",
		".dir arg1,arg2",
		".dir -0xff, hello",
		"ld 0x01, %r0",
		"csrrd %status, %r1"
	};
	Instruction* ins = nullptr;
	Directive* dir = nullptr;
	std::string label = "";
	Parser parser;

	for (int i = 0; i < 11; i++) {
		result += std::to_string(i) + ". ";
		try {
			parser.parseAssemblerLine(lines[i], &label, &dir, &ins);
			if (label != "") result += label + ": ";
			if (ins) result += ins->str();
			if (dir) result += dir->str();
			result += "\n";
		}
		catch (Parser::ParserException& e) {
			std::cout << e.getMsg() << std::endl;
		}
		if (ins) delete ins;
		if (dir) delete dir;
		ins = nullptr; dir = nullptr; label = "";
	}

	std::string correct = "0. label1: beq %r15, %r15, $-15\n1. lab2: \n2. ld [%r0 + symbol1], %r4\n3. st %r9, [%r5 + -15]\n4. ld [%r4], %r4\n";
	correct += "5. ld $256, %r4\n6. ld $immsym, %r3\n7. .dir arg1, arg2\n8. .dir -255, hello\n9. ld 1, %r0\n10. csrrd %r0, %r1\n";

	std::cout << "RESULT:\n" << result << std::endl;
	std::cout << "CORRECT:\n" << correct << std::endl;
	std::cout << "TEST PASSED: " << (result == correct ? "Yes" : "No") << std::endl;
}

void Tests::testSystem1() {
	std::unordered_map<std::string, int>* locs = new std::unordered_map<std::string, int>();
	locs->insert({ ".text", 0x40000000 });

	std::string files[] = { "Test1-A", "Test1-B" };
	std::string result = testSystem(files, 2, locs);

	std::cout << "Emulator output: " + result << std::endl;
	std::string correct = "b";
	std::cout << "Correct output: " << correct << std::endl;
	std::cout << "TEST PASSED: " << (correct == result ? "Yes" : "No") << std::endl;

	delete locs;
}

void Tests::testSystem2() {
	std::unordered_map<std::string, int>* locs = new std::unordered_map<std::string, int>();
	locs->insert({ ".main", 0x40000000 });

	std::string files[] = { "Test2-A", "Test2-B", "Test2-C"};
	std::string result = testSystem(files, 3, locs);

	std::cout << "Emulator output: " + result << std::endl;
	std::string correct = "abcdefghijklmn";
	std::cout << "Correct output: " << correct << std::endl;
	std::cout << "TEST PASSED: " << (correct == result ? "Yes" : "No") << std::endl;

	delete locs;
}
void Tests::testSystem3() {
	std::unordered_map<std::string, int>* locs = new std::unordered_map<std::string, int>();
	locs->insert({ ".main", 0x40000000 });

	std::string files[] = { "Test3-A", "Test3-B", "Test3-C" };
	std::string result = testSystem(files, 3, locs);

	std::cout << "Emulator output: " + result << std::endl;
	std::string correct = "inside";
	std::cout << "Correct output: " << correct << std::endl;
	std::cout << "TEST PASSED: " << (correct == result ? "Yes" : "No") << std::endl;

	delete locs;
}

std::string Tests::testSystem(std::string files[], int cnt, std::unordered_map<std::string, int>* locs) {
	std::string result = "";
	std::ifstream* assemblerInput = nullptr;
	std::ifstream** linkerInputs = new std::ifstream * [cnt];

	Assembler assembler;
	for (int i = 0; i < cnt; i++) {
		assemblerInput = new std::ifstream(files[i] + ".txt");

		std::string obj = assembler.assemble(assemblerInput);
		assemblerInput->close();
		delete assemblerInput; assemblerInput = nullptr;

		std::string objName = files[i] + ".obj";
		result += "Assembler file " + objName + ":\n" + obj + "\n";
		std::ofstream out(objName);
		out << obj;
		out.close();

		linkerInputs[i] = new std::ifstream(objName);
	}

	Linker linker;
	std::string objMerged = linker.merge(linkerInputs, cnt);
	result += "Merged file:\n" + objMerged + "\n";
	std::ofstream out(files[0].substr(0, 5) + "-Merge.obj");
	out << objMerged;
	out.close();

	for (int i = 0; i < cnt; i++) linkerInputs[i]->seekg(0);
	std::string exe = linker.link(linkerInputs, cnt, locs);
	result += "Exe file:\n" + exe + "\n";
	std::string exeName = files[0].substr(0, 5) + ".exe";
	out = std::ofstream(exeName);
	out << exe;
	out.close();

	for (int i = 0; i < cnt; i++) {
		linkerInputs[i]->close();
		delete linkerInputs[i];
		linkerInputs[i] = nullptr;
	}
	delete[] linkerInputs; linkerInputs = nullptr;

	std::ifstream* emulatorInput = new std::ifstream(exeName);
	std::string emulatorOutput = "";
	Emulator emulator(emulatorInput, &emulatorOutput);
	emulatorInput->close(); delete emulatorInput; emulatorInput = nullptr;
	emulator.emulate();

	std::cout << result;

	return emulatorOutput;
}

