#include "../inc/Emulator.h"
#include <regex>
#include <fstream>
#include <termios.h>
#include <unistd.h>
#include <iomanip>

int Emulator::execute(InstructionCode ic) {
	int oc = ic.getOC();
	int pc = regs.getPC();
	int sp = regs.getSP();
	int regA = ic.getA(); int a = regs.get(regA);
	int regB = ic.getB(); int b = regs.get(regB);
	int regC = ic.getC(); int c = regs.get(regC);
	int d = ic.getDisplacement();
	int ret = NO_CAUSE;
	if (oc == 0x00) {
		start = false;
	}
	else if ((oc & 0xF0) == 0x20) {
		int addr = a + b + d;
		if (oc & 0xF) addr = mem.getInt(addr);
		sp -= 4;
		mem.writeInt(sp, pc);
		pc = addr;
		regs.setPC(pc);
		regs.setSP(sp);
	}
	else if ((oc & 0xf0) == 0x30) {
		int addr = a + d;
		if (oc & 0x8) addr = mem.getInt(addr);
		if ((oc & 0x7) == 0 || (oc & 0x7) == 1 && b == c || (oc & 0x7) == 2 && b != c || (oc & 0x7) == 3 && b > c) {
			pc = addr;
			regs.setPC(pc);
		}
	}
	else if (oc == 0x40) {
		int tmp = b; b = c; c = tmp;
		regs.set(regB, b);
		regs.set(regC, c);
	}
	else if (oc == 0x50) {
		a = b + c;
		regs.set(regA, a);
	}
	else if (oc == 0x51) {
		a = b - c;
		regs.set(regA, a);
	}
	else if (oc == 0x52) {
		a = b * c;
		regs.set(regA, a);
	}
	else if (oc == 0x53) {
		a = b / c;
		regs.set(regA, a);
	}
	else if (oc == 0x60) {
		a = ~b;
		regs.set(regA, a);
	}
	else if (oc == 0x61) {
		a = b & c;
		regs.set(regA, a);
	}
	else if (oc == 0x62) {
		a = b | c;
		regs.set(regA, a);
	}
	else if (oc == 0x63) {
		a = a ^ c;
		regs.set(regA, a);
	}
	else if (oc == 0x70) {
		a = b << c;
		regs.set(regA, a);
	}
	else if (oc == 0x71) {
		a = b >> c;
		regs.set(regA, a);
	}
	else if (oc == 0x80) {
		int addr = a + b + d;
		mem.writeInt(addr, c);
	}
	else if (oc == 0x81) {
		a += d;
		regs.set(regA, a);
		mem.writeInt(a, c);
	}
	else if (oc == 0x82) {
		int addr = a + b + d;
		addr = mem.getInt(addr);
		mem.writeInt(addr, c);
	}
	else if (oc == 0x90) {
		a = regs.getCSR(regB);
		regs.set(regA, a);
	}
	else if (oc == 0x94) {
		regs.setCSR(regA, b);
	}
	else if (oc == 0x91) {
		a = b + d;
		regs.set(regA, a);
	}
	else if (oc == 0x92) {
		a = mem.getInt(b + c + d);
		regs.set(regA, a);
	}
	else if (oc == 0x96) {
		regs.setCSR(regA, mem.getInt(b + c + d));
	}
	else if (oc == 0x93) {
		a = mem.getInt(b);
		b += d;
		regs.set(regA, a);
		regs.set(regB, b);
	}
	else if (oc == 0x97) {
		regs.setCSR(regA, mem.getInt(b));
		b += d;
		regs.set(regB, b);
	}
	else if (oc == 0x95) {
		regs.setCSR(regA, regs.getCSR(regB) | d);
	}
	else {
		if (oc == 0x10) ret = CAUSE_INT; //intr
		else ret = CAUSE_BAD_CODE; //incorrect instruction code
	}
	return ret;
}
Emulator::Emulator(std::ifstream* initData) {
	regs.setPC(0x40000000);
	this->output = nullptr;
	if (!initData) return;
	std::string line;
	while (std::getline(*initData, line)) {
		if (line == "") continue;
		std::regex pattern(R"del(^(\w+): ((?:\w\w)|  ) ((?:\w\w)|  ) ((?:\w\w)|  ) ((?:\w\w)|  ) ((?:\w\w)|  ) ((?:\w\w)|  ) ((?:\w\w)|  ) ((?:\w\w)|  )$)del");
		std::sregex_iterator iter = std::sregex_iterator(line.begin(), line.end(), pattern);
		std::sregex_iterator end;
		if (iter != end) {
			std::smatch match = *iter;
			unsigned addr = std::stoul(match[0].str(), nullptr, 16);
			for (int i = 2; i <= 9; i++) {
				std::string val = match[i].str();
				if (val != "  ") mem.writeByte(addr + i - 2, std::stoi(val, nullptr, 16));
			}
		}
	}
}
Emulator::Emulator(std::ifstream* initData, std::string* output) : Emulator(initData) {
	this->output = output;
}
void Emulator::emulate() {
	initTerminal();
	int instructionsSinceLastTimeInterrupt = 0;
	int instructionsSinceLastCharInput = 0;
	int instructionsPerMillisecond = 200;

	while (start) {
		//Read next instruction.
		int pc = regs.getPC(); regs.setPC(pc + 4);
		InstructionCode ins = InstructionCode(mem.getInt(pc));
		//printf("Read instruction %x from address %x.\n", ins.getOC(), regs.getPC());
		//Execute instruction.
		int cause = execute(ins);
		instructionsSinceLastCharInput++;
		instructionsSinceLastTimeInterrupt++;
		//Check if a character was written.
		if (mem.termOutWasWritten()) putChar();
		//Check for new interrupt signals
		if (cause != NO_CAUSE) interrupts[cause] = true;
		if (charReady() && instructionsSinceLastCharInput > 20) {
			instructionsSinceLastCharInput = 0;
			interrupts[CAUSE_TERMINAL] = true;
			mem.writeTermIn(getChar());
		}
		if (instructionsSinceLastTimeInterrupt >= getPeriod() * instructionsPerMillisecond) {
			instructionsSinceLastTimeInterrupt = 0;
			interrupts[CAUSE_TIMER] = true;
		}
		//Process interrupt signals
		cause = getHighestPriorityInterrupt();
		if (cause != NO_CAUSE) {
			interrupts[cause] = false;
			int sp = regs.getSP();
			sp -= 4;
			mem.writeInt(sp, regs.getPC());
			sp -= 4;
			mem.writeInt(sp, regs.getStatus());
			regs.setSP(sp);
			regs.setI();
			regs.setCause(cause);
			regs.setPC(regs.getHandler());
		}
	}
	restoreTerminal();
	printEnd();
}
int Emulator::getPeriod() {
	int val = mem.getTimeConfig();
	if (val <= 3 && val >= 0) return (val + 1) * 500;
	else if (val == 4) return 5000;
	else if (val == 5) return 10000;
	else if (val == 6) return 30000;
	else if (val == 7) return 60000;
	else return 500;
}
void Emulator::initTerminal() {
	struct termios settings;
	tcgetattr(STDIN_FILENO, &oldTerminalSettings);
	settings = oldTerminalSettings;
	settings.c_lflag &= ~(ICANON | ECHO);
	settings.c_cc[VTIME] = 0;
	settings.c_cc[VMIN] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &settings);
}
bool Emulator::charReady() {
	if (input != -1) return true;
	char c;
	if (read(STDIN_FILENO, &c, 1) > 0) {
		input = c;
		return true;
	}
	else return false;
}
char Emulator::getChar() {
	char ret = input;
	input = -1;;
	return ret;
}
void Emulator::putChar() {
	char c = (char)mem.getTermOut();
	if (output) *output += c;
	else std::cout << c << std::flush;
	return;
}
void Emulator::restoreTerminal() {
	tcsetattr(STDIN_FILENO, TCSANOW, &oldTerminalSettings);
}
int Emulator::getHighestPriorityInterrupt() {
	if (interrupts[CAUSE_BAD_CODE]) return CAUSE_BAD_CODE;
	if (interrupts[CAUSE_INT]) return CAUSE_INT;
	if (interrupts[CAUSE_TERMINAL] && !regs.getI() && ! regs.getT1()) return CAUSE_TERMINAL;
	if (interrupts[CAUSE_TIMER] && !regs.getI() && !regs.getTr()) return CAUSE_TIMER;
	return NO_CAUSE;
}

void Emulator::printEnd() {
	std::cout << "Emulator execution done. Final state:" << std::endl << std::endl;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			int reg = i*4 + j;
			unsigned val = regs.get(reg);
			std::cout << "\tr" << reg << ": 0x" << std::hex << std::setw(8) << std::setfill('0') << val;
		}
		std::cout << std::endl;
	}
}