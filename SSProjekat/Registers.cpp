#include "Registers.h"

int Registers::get(int i) {
	if (i) return r[i-1];
	else return 0;
}
void Registers::set(int i, int num) {
	if (i) r[i-1] = num;
}
int Registers::getSP() {
	return get(14);
}
void Registers::setSP(int num) {
	set(14, num);
}
int Registers::getPC() {
	return get(15);
}
void Registers::setPC(int num) {
	set(15, num);
}
int Registers::getHandler() {
	return handler;
}
void Registers::setHandler(int num) {
	handler = num;
}
int Registers::getStatus() {
	return status;
}
void Registers::setStatus(int num) {
	status = num;
}
int Registers::getI() {
	return (status & 4) >> 2;
}
int Registers::getT1() {
	return (status & 2) >> 1;
}
int Registers::getTr() {
	return status & 1;
}
void Registers::setI() {
	status |= 4;
}
void Registers::setT1() {
	status |= 2;
}
void Registers::setTr() {
	status |= 1;
}
void Registers::clearI() {
	status &= ~4;
}
void Registers::clearT1() {
	status &= ~2;
}
void Registers::clearTr() {
	status &= ~1;
}
int Registers::getCause() {
	return cause;
}
void Registers::setCause(int num) {
	cause = num;
}
int Registers::getCSR(int i) {
	if (i == 0) return getStatus();
	else if (i == 1) return getHandler();
	else if (i == 2) return getCause();
	else return 0;
}
void Registers::setCSR(int i, int num) {
	if (i == 0) setStatus(num);
	else if (i == 1) setHandler(num);
	else if (i == 2) setCause(num);

}