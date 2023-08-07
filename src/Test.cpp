#include "../inc/Parser.h"
#include <iostream>
#include "../inc/Tests.h"
#include "../inc/Assembler.h"
#include <sstream>
#include "../inc/Exception.h"
#include "../inc/Linker.h"
#include "../inc/Emulator.h"
#include "../inc/Memory.h"
#include "../inc/EquDirective.h"
#include <chrono>
#include <thread>
#define TEST_DIR std::string("tests/")

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

	std::string comments[] = { "hello #comment\n", "hello #", "#entirelineiscomment", "no comment here"};
	for (int i = 0; i < 4; i++) result += parser.removeComment(comments[i]) + "\n";

	std::string asciiString = " before\"ascii\\t\\\\string\"after";
	result += parser.parseAsciiString(asciiString) + "\n";
	

	std::string correct = "0. label1: beq %r15, %r15, $-15\n1. lab2: \n2. ld [%r0 + symbol1], %r4\n3. st %r9, [%r5 + -15]\n4. ld [%r4], %r4\n";
	correct += "5. ld $256, %r4\n6. ld $immsym, %r3\n7. .dir arg1, arg2\n8. .dir -255, hello\n9. ld 1, %r0\n10. csrrd %r0, %r1\nhello \nhello \n\nno comment here\nascii	\\string\n";

	std::cout << "RESULT:\n" << result << std::endl;
	std::cout << "CORRECT:\n" << correct << std::endl;
	std::cout << "TEST PASSED: " << (result == correct ? "Yes" : "No") << std::endl;
}

void Tests::testEmulatorTerminal() {
	Emulator emulator(nullptr);
	std::cout << "Correct behaviour: should read 10 characters from stdin, without blocking, and without waiting for end of line." << std::endl;
	emulator.initTerminal();
	int read = 0;
	while (read < 10) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		if (emulator.charReady()) {
			std::cout << "Input char: " << emulator.getChar() << std::endl;
			read++;
		}
		else {
			std::cout << "No char still." << std::endl;
		}
	}
	emulator.restoreTerminal();
}

void Tests::testSystem1() {
	std::unordered_map<std::string, int>* locs = new std::unordered_map<std::string, int>();
	locs->insert({ ".text", 0x40000000 });

	std::string files[] = { "Test1-A", "Test1-B" };
	testSystem(files, 2, locs, "b");

	delete locs;
}

void Tests::testSystem2() {
	std::unordered_map<std::string, int>* locs = new std::unordered_map<std::string, int>();
	locs->insert({ ".main", 0x40000000 });

	std::string files[] = { "Test2-A", "Test2-B", "Test2-C"};
	testSystem(files, 3, locs, "abcdefghijklmn");

	delete locs;
}
void Tests::testSystem3() {
	std::unordered_map<std::string, int>* locs = new std::unordered_map<std::string, int>();
	locs->insert({ ".main", 0x40000000 });

	std::string files[] = { "Test3-A", "Test3-B", "Test3-C" };
	testSystem(files, 3, locs, "insideinside");

	delete locs;
}
void Tests::testSystem4() {
	std::unordered_map<std::string, int>* locs = new std::unordered_map<std::string, int>();
	locs->insert({ ".text", 0x40000000 });

	std::string files[] = { "Test4-A" };
	testSystem(files, 1, locs, "Some \\stringa");

	delete locs;
}
void Tests::testSystem5() {
	std::unordered_map<std::string, int>* locs = new std::unordered_map<std::string, int>();
	locs->insert({ ".text", 0x40000000 });

	std::string files[] = { "Test5-A", "Test5-B"};
	testSystem(files, 2, locs, "helo helo");

	delete locs;
}
void Tests::testSystem6() {
	std::unordered_map<std::string, int>* locs = new std::unordered_map<std::string, int>();
	locs->insert({".main", 0x40000000});

	std::string files[] = {"Test6-A", "Test6-B", "Test6-C"};
	testSystem(files, 3, locs, "stdout");

	delete locs;
}

void Tests::testSystem7() {
	std::unordered_map<std::string, int>* locs = new std::unordered_map<std::string, int>();
	locs->insert({".main", 0x40000000});
	locs->insert({".func", 0x50000000});

	std::string files[] = {"Test7-A", "Test7-B"};
	testSystem(files, 2, locs, "a");

	delete locs;
}

void Tests::testSystem8() {
	std::unordered_map<std::string, int>* locs = new std::unordered_map<std::string, int>();
	locs->insert({".main", 0x40000000});

	std::string files[] = {"Test8-A", "Test8-B", "Test8-C", "Test8-D"};
	testSystem(files, 4, locs, "stdout");

	delete locs;
}

void Tests::testSystem9() {
	std::string files1[] = {"tests/Test8-A.txt", "tests/Test8-B.txt"};
	std::string files2[] = {"tests/Test8-C.txt", "tests/Test8-D.txt"};
	std::string* parts[] = {files1, files2};
	Assembler assembler;
	Linker linker;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			std::ifstream input(parts[i][j]);
			std::string objFile = assembler.assemble(&input);
			input.close();
			std::string outputName = "tests/Test9-" + std::to_string(i*2 + j) + ".obj";
			std::cout << "Assembler output " << outputName << ":" << std::endl;
			std::cout << objFile << std::endl;
			std::ofstream output(outputName);
			output << objFile;
		}
		std::ifstream** inputs = new std::ifstream*[2];
		for (int j = 0; j < 2; j++) inputs[j] = new std::ifstream("tests/Test9-" + std::to_string(i*2 + j) + ".obj");
		std::string mergedFile = linker.merge(inputs, 2);
		for (int i = 0; i < 2; i++) {
			inputs[i]->close(); delete inputs[i]; 
		}
		delete inputs;
		std::string outputName = "tests/Test9-Part" + std::to_string(i) + ".obj";
		std::cout << "Merged file: " << outputName << ":" << std::endl;
		std::cout << mergedFile << std::endl;
		std::ofstream output(outputName);
		output << mergedFile;
		output.close();
	}
	std::ifstream** inputs = new std::ifstream*[2];
	for (int i = 0; i < 2; i++) inputs[i] = new std::ifstream("tests/Test9-Part" + std::to_string(i) + ".obj");
	std::unordered_map<std::string, int>* locs = new std::unordered_map<std::string, int>();
	locs->insert({".main", 0x40000000});
	std::string exeFile = linker.link(inputs, 2, locs);
	delete locs; for (int i = 0; i < 2; i++) {
		inputs[i]->close();
		delete inputs[i];
	}
	delete inputs;
	std::cout << "Executable: " << std::endl;
	std::cout << exeFile << std::endl;
	std::ofstream output("tests/Test9.exe");
	output << exeFile;
	output.close();

	std::ifstream emuInput("tests/Test9.exe");
	Emulator emulator(&emuInput);
	emuInput.close();
	emulator.emulate();

	std::cout << "Test finished" << std::endl;
}

void Tests::testSystem(std::string files[], int cnt, std::unordered_map<std::string, int>* locs, std::string correctEmulatorOutput) {
	std::string result = "";
	std::ifstream* assemblerInput = nullptr;
	std::ifstream** linkerInputs = new std::ifstream * [cnt];

	Assembler assembler;
	for (int i = 0; i < cnt; i++) {
		assemblerInput = new std::ifstream(TEST_DIR + files[i] + ".txt");

		std::string obj = assembler.assemble(assemblerInput);
		assemblerInput->close();
		delete assemblerInput; assemblerInput = nullptr;

		std::string objName = TEST_DIR + files[i] + ".obj";
		result += "Assembler file " + objName + ":\n" + obj + "\n";
		std::ofstream out(objName);
		out << obj;
		out.close();

		linkerInputs[i] = new std::ifstream(objName);
	}

	Linker linker;
	std::string objMerged = linker.merge(linkerInputs, cnt);
	result += "Merged file:\n" + objMerged + "\n";
	std::ofstream out(TEST_DIR + files[0].substr(0, 5) + "-Merge.obj");
	out << objMerged;
	out.close();

	for (int i = 0; i < cnt; i++) linkerInputs[i]->seekg(0);
	std::string exe = linker.link(linkerInputs, cnt, locs);
	result += "Exe file:\n" + exe + "\n";
	std::string exeName = TEST_DIR + files[0].substr(0, 5) + ".exe";
	out = std::ofstream(exeName);
	out << exe;
	out.close();

	for (int i = 0; i < cnt; i++) {
		linkerInputs[i]->close();
		delete linkerInputs[i];
		linkerInputs[i] = nullptr;
	}
	delete[] linkerInputs; linkerInputs = nullptr;

	if (correctEmulatorOutput == "stdout") {
		std::ifstream* emulatorInput = new std::ifstream(exeName);
		Emulator emulator(emulatorInput);
		emulatorInput->close(); delete emulatorInput; emulatorInput = nullptr;
		emulator.emulate();

		std::cout << result;
	}
	else {
		std::ifstream* emulatorInput = new std::ifstream(exeName);
		std::string emulatorOutput = "";
		Emulator emulator(emulatorInput, &emulatorOutput);
		emulatorInput->close(); delete emulatorInput; emulatorInput = nullptr;
		emulator.emulate();

		std::cout << result;

		std::cout << "Emulator output: " + emulatorOutput << std::endl;
		std::cout << "Correct output: " << correctEmulatorOutput << std::endl;
		std::cout << "TEST PASSED: " << (correctEmulatorOutput == emulatorOutput ? "Yes" : "No") << std::endl;
	}
}

void Tests::testEquDirective() {
	Parser parser;
	std::vector<Operand*>* ops = new std::vector<Operand*>();
	ops->push_back(new MemorySymbolOperand("TestSymbol"));
	ops->push_back(new SpecialOperand("sym1 + 5 - sym2 + sym3 - 2"));
	Directive* dir = new Directive("equ", ops);
	EquDirective equ(dir);
	delete dir; dir = nullptr;

	std::string result = equ.str();
	SymbolTable symTab;
	symTab.addSymbol(100, 'O', "sec1", "sym1");
	bool test1 = equ.canBeResolved(&symTab);
	result += test1 ? "yes" : "no";
	result += "\n";

	symTab.addSymbol(50, 'O', "sec1", "sym2");
	symTab.addSymbol(150, 'O', "sec2", "sym3");
	bool test2 = equ.canBeResolved(&symTab);
	result += test2 ? "yes" : "no";
	result += "\n";

	if (test2) equ.resolve(&symTab);
	
	result += symTab.str() + "\n";

	std::cout << "RESULT:\n\n" << result;

	std::string correct = "Name: TestSymbol\n"
		"Literal: 3\n"
		"Add syms: sym1 sym3\n"
		"Sub syms: sym2\n"
		"no\n"
		"yes\n"
		"SYMTAB\n"
		"203,O,L,sec2,TestSymbol\n"
		"100,O,L,sec1,sym1\n"
		"50,O,L,sec1,sym2\n"
		"150,O,L,sec2,sym3\n";

	std::cout << "CORRECT:\n\n" << correct;

	std::cout << "\nTEST PASSED: " << (correct == result ? "Yes" : "No");
}