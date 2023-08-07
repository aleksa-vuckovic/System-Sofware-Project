#ifndef _MEMORY_H_
#define _MEMORY_H_
#include <unordered_map>
/*
* An object of class Memory represents byte addressible memory space with 32 bit addreses.
* The Terminal Input Register, Terminal Output Register and Time Configuration Register are special memory locations.
* Integers are written in little endian format.
*/
class Memory {
	static const int segSize = 1 << 12;
	std::unordered_map<unsigned, char*> data;
	/*
	* Set whenever the term out register is written (by the user program).
	* Unset when the termInWasWritten method is called (by the emulator).
	*/
	bool termOutWritten = false;
	/*
	* Unset whenever the writeTermInMethod is called (by the emulator).
	* Set whenever the term in register is read (by the user program).
	*/
	//bool termInRead = true;
public:
	Memory();
	~Memory();
	unsigned char getByte(unsigned addr);
	unsigned int getInt(unsigned addr);
	void writeByte(unsigned addr, unsigned char byte);
	//Sets the termOutWritten flag if that memory address is written.
	void writeInt(unsigned addr, unsigned int num);
	//Returns the termOutWritten flag, and resets it.
	bool termOutWasWritten();
	int getTermOut();
	void writeTermIn(char c);
	//bool termInWasRead();
	int getTimeConfig();
};

#endif