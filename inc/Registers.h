#ifndef _REGISTERS_H_
#define _REGISTERS_H_

/*
* Represents the set of registers on the emulated system.
*/
class Registers {
	int r[15];
	int handler, status, cause;
public:
	int get(int i);
	void set(int i, int num);
	int getSP();
	void setSP(int num);
	int getPC();
	void setPC(int num);

	int getHandler();
	void setHandler(int num);
	int getStatus();
	void setStatus(int num);
	int getI();
	int getT1();
	int getTr();
	void setI();
	void setT1();
	void setTr();
	void clearI();
	void clearT1();
	void clearTr();
	int getCause();
	void setCause(int num);

	int getCSR(int i);
	void setCSR(int i, int num);
};

#endif