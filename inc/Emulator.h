#ifndef _EMULATOR_H_
#define _EMULATOR_H_
#include "Memory.h"
#include "Registers.h"
#include "InstructionCode.h"
#include "Exception.h"
#include <iostream>
#include <termios.h>
#define NO_CAUSE 0
#define CAUSE_BAD_CODE 1
#define CAUSE_TIMER 2
#define CAUSE_TERMINAL 3
#define CAUSE_INT 4


/*
* This class represents ~The Emulator~.
*/
class Emulator {
public:
	class EmulatorException : public Exception {
	public:
		EmulatorException(std::string msg) : Exception(msg) {}
	};
private:
	Memory mem;
	Registers regs;
	bool start = true;
	std::string* output = nullptr;
	char input = -1;
	struct termios oldTerminalSettings;
	bool interrupts[5];
	/*
	* Returns the highest priority interrupt signal which is set to true in interrupts[],
	* or NO_CAUSE, if no interrupt signal is active.
	*/
	int getHighestPriorityInterrupt();
	/*
	* This method executes the instruction, modifying the relevan registers and memory locations.
	* However, if an interrupt happens, for example because of a bad instruction code, or an interrupt
	* instruction, this method does not push the pc and status registers, it just returns the cause 
	* of the interrupt as an integer. If no interrupt occured, -1 is returned.
	*/
	int execute(InstructionCode ic);
	/*
	* Returns the period of clock interrupts in milliseconds, as determined by the Time Conficuration Register.
	*/
	int getPeriod();
	//Terminal methods
	void initTerminal();
	bool charReady();
	char getChar();
	void putChar();
	void restoreTerminal();
	void printEnd();

	friend class Tests;
public:
	/*
	* The constructor initializes the memory based on the input file,
	* which should be in the format produced by Linker::link().
	* The PC register is always initialized to 0x40000000.
	*/
	Emulator(std::ifstream* initData);
	/*
	* An emulator constructed with this constructer redirects its output
	* to the output string. This is useful for testing purposes.
	*/
	Emulator(std::ifstream* initData, std::string* output);
	
	void emulate();
};


#endif