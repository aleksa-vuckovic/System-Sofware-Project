#include "Parser.h"
#include <iostream>
#include "Tests.h"
#include "Assembler.h"
#include <sstream>
#include "Exception.h"
#include "Linker.h"
#include "Emulator.h"
#include "Memory.h"

void testConverter() {
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
void testMemory() {
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
void testParser() {
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
		"ld 0x01, %r0"
	};
	Instruction* ins = nullptr;
	Directive* dir = nullptr;
	std::string label = "";
	Parser parser;

	for (int i = 0; i < 10; i++) {
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
	correct += "5. ld $256, %r4\n6. ld $immsym, %r3\n7. .dir arg1, arg2\n8. .dir -255, hello\n9. ld 1, %r0\n";

	std::cout << "RESULT:\n" << result << std::endl;
	std::cout << "CORRECT:\n" << correct << std::endl;
	std::cout << "TEST PASSED: " << (result == correct ? "Yes" : "No") << std::endl;
}


void testAssembler() {
	Assembler assembler;

	std::string program =
		".section .text\n"
		"push %r0\n"
		"ld 0x01, %r0\n"
		"st %r1, var\n"
		".section .data\n"
		"var: .word -1\n"
		".end";
	std::istringstream input = std::istringstream(program);
	try {
		std::string result = assembler.assemble(input);
		std::cout << result;
	}
	catch (Exception e) {
		std::cout << e.getMsg() << std::endl;
	}
	
}

void testLinker() {
	std::string program1 =
		".extern f\n"
		".global var\n"
		".section .text\n"
		"ld $0x100, %sp\n"
		"call f\n"
		"ld var, %r1\n"
		"st %r1, 0xFFFFFF00\n"
		"halt\n"
		".section .data\n"
		"var: .word 1\n";
	std::string program2 =
		".extern var\n"
		".global f\n"
		".section .text\n"
		"f: ld $97, %r1\n"
		"ld var, %r2\n"
		"add %r2, %r1\n"
		"st %r1, var\n"
		"ret\n";
	
	Assembler assembler;
	std::istringstream input1 = std::istringstream(program1);
	std::istringstream input2 = std::istringstream(program2);
	std::string obj[] = { assembler.assemble(input1), assembler.assemble(input2) };

	std::string files[] = { "obj1.txt", "obj2.txt" };
	for (int i = 0; i < 2; i++) {
		std::cout << "Assembler file " + std::to_string(i) + ":\n" + obj[i] << std::endl;
		if (std::ifstream(files[i])) std::remove(files[i].c_str());
		std::ofstream out(files[i]);
		out << obj[i];
		out.close();
	}

	Linker linker;
	std::ifstream inputs[] = { std::ifstream(files[0]), std::ifstream(files[1]) };
	std::string objMerged = linker.merge(inputs, 2);
	inputs[0].seekg(0); inputs[1].seekg(0);
	std::cout << "Merged file:\n" + objMerged << std::endl;

	std::unordered_map<std::string, int> locs;
	locs.insert({ ".text", 0x40000000 });
	std::string exe = linker.link(inputs, 2, &locs);
	std::cout << "Exe file:\n" + exe << std::endl;
	inputs[0].close(); inputs[1].close();

	std::istringstream exeStream = std::istringstream(exe);
	Emulator emulator(exeStream);
	emulator.emulate();
}