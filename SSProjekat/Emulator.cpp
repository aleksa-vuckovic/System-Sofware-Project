#include "Emulator.h"
#include <regex>


int Emulator::execute(InstructionCode ic) {
	int oc = ic.getOC();
	int pc = regs.getPC();
	int sp = regs.getSP();
	int regA = ic.getA(); int a = regs.get(regA);
	int regB = ic.getB(); int b = regs.get(regB);
	int regC = ic.getC(); int c = regs.get(regC);
	int d = ic.getDisplacement();
	int ret = -1;
	if (oc == 0x00) {
		start = false;
	}
	else if (oc == 0x20) {
		int addr = a + b + d;
		sp -= 4;
		mem.writeInt(sp, pc);
		pc = addr;
		regs.setPC(pc);
		regs.setSP(sp);
	}
	else if ((oc & 0xf0) == 0x30) {
		int addr = a + d;
		if ((oc & 0xf) == 0 || (oc & 0xf) == 1 && b == c || (oc & 0xf) == 2 && b != c || (oc & 0xf) == 3 && b > c) {
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
	else if (oc == 0x90) {
		a = regs.getCSR(regB);
		regs.set(regC, a);
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
		if (oc == 0x10) ret = 4; //intr
		else ret = 1; //incorrect instruction code
	}
	return ret;
}
Emulator::Emulator(std::istream& initData) {
	std::string line;
	while (std::getline(initData, line)) {
		if (line == "") continue;
		std::regex pattern(R"del(^(\d+): ((?:\w\w)|  ) ((?:\w\w)|  ) ((?:\w\w)|  ) ((?:\w\w)|  ) ((?:\w\w)|  ) ((?:\w\w)|  ) ((?:\w\w)|  ) ((?:\w\w)|  )$)del");
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
	regs.setPC(0x40000000);
}
void Emulator::emulate() {
	int instructionsSinceLastTimeInterrupt = 0;
	int instructionsSinceLastTerminalInterrupt = 0;
	int instructionsPerMillisecond = 1000000;
	while (start) {
		//Read next instruction.
		int pc = regs.getPC(); regs.setPC(pc + 4);
		InstructionCode ins = InstructionCode(mem.getInt(pc));
		//printf("Read instruction %x from address %x.", ins.getOC(), regs.getPC());
		//Execute instruction.
		int cause = execute(ins);
		instructionsSinceLastTerminalInterrupt++;
		instructionsSinceLastTimeInterrupt++;
		//Check if a character was written.
		if (mem.termOutWasWritten()) {
			putChar();
		}
		//check for terminal or time interrupt
		if (cause == -1) {
			if (charReady() && !regs.getI() && !regs.getT1() && instructionsSinceLastTerminalInterrupt > 5) {
				//The conditions have to be checked before writing the char.
				mem.writeTermIn(getChar());
				instructionsSinceLastTerminalInterrupt = 0;
				cause = 3;
			}
			else if (instructionsSinceLastTimeInterrupt / instructionsPerMillisecond >= getPeriod() && !regs.getI() && !regs.getTr()) {
				instructionsSinceLastTimeInterrupt = 0;
				cause = 2;
			}
		}
		if (cause != -1 && !regs.getI()) {
			int sp = regs.getSP();
			sp -= 4;
			mem.writeInt(sp, regs.getStatus());
			sp -= 4;
			mem.writeInt(sp, regs.getPC());
			regs.setSP(sp);
			regs.setI();
			regs.setCause(cause);
			regs.setPC(regs.getHandler());
		}
	}
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

}
bool Emulator::charReady() {
	return false;
}
char Emulator::getChar() {
	return 0;
}
void Emulator::putChar() {
	std::cout << std::endl << "Char printed: " << (char)mem.getTermOut() << std::endl;
	return;
}