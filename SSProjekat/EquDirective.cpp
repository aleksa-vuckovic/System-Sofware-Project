#include "EquDirective.h"
#include <regex>
#include "Parser.h"
#include "Exception.h"
#include <iostream>
#include <unordered_map>
#include "Exception.h"
std::string EquDirective::getNextPart(std::string& expr) {
	std::regex pattern(R"del(^\s*([^\s]+)(.*)$)del");
	std::sregex_iterator iter = std::sregex_iterator(expr.begin(), expr.end(), pattern);
	std::sregex_iterator end;
	if (iter != end) {
		std::smatch match = *iter;
		std::string g1 = match[1].str();
		std::string g2 = match[2].str();
		expr = g2;
		return g1;
	}
	else {
		expr = "";
		return "";
	}
}
long long EquDirective::getLiteral(std::string op) {
	Parser parser;
	return parser.parseLiteral(op);
}
EquDirective::EquDirective(Directive* dir) {
	symbol = dir->getOperand(0)->getSymbol();
	Operand* operand = dir->getOperand(1);
	if (operand->getType() == Operand::MEM_LIT) {
		literal = operand->getLiteral();
		return;
	}
	else if (operand->getType() == Operand::MEM_SYM) {
		addSyms.push_back(operand->getSymbol());
		return;
	}
	std::string expression = operand->getOriginalString();
	literal = 0;
	std::string part;
	char op = '+';
	while ((part = getNextPart(expression)) != "") {
		if (part == "+") op = '+';
		else if (part == "-") op = '-';
		else try {
			long long lit = getLiteral(part);
			if (op == '+') literal += lit;
			else literal -= lit;
		}
		catch (Exception e) {
			if (op == '+') addSyms.push_back(part);
			else subSyms.push_back(part);
		}
	}
}
bool EquDirective::canBeResolved(SymbolTable* symTab) {
	for (auto it = addSyms.begin(); it != addSyms.end(); it++) if (!symTab->isDefined(*it)) return false;
	for (auto it = subSyms.begin(); it != subSyms.end(); it++) if (!symTab->isDefined(*it)) return false;
	return true;
}
void EquDirective::resolve(SymbolTable* symTab) {
	std::unordered_map<std::string, int> sectionCnt;
	for (auto it = addSyms.begin(); it != addSyms.end(); it++) {
		SymbolTable::Entry* entry = symTab->getEntry(*it);
		literal += entry->value;
		std::string sec = entry->section;
		if (sec == "*ABS*") continue;
		if (sectionCnt.count(sec) == 0) sectionCnt.insert({ sec, 0 });
		sectionCnt[sec]++;
	}
	for (auto it = subSyms.begin(); it != subSyms.end(); it++) {
		SymbolTable::Entry* entry = symTab->getEntry(*it);
		literal -= entry->value;
		std::string sec = entry->section;
		if (sec == "*ABS*") continue;
		if (sectionCnt.count(sec) == 0) sectionCnt.insert({ sec, 0 });
		sectionCnt[sec]--;
	}
	std::string section = "";
	for (auto it = sectionCnt.begin(); it != sectionCnt.end(); it++) {
		if (it->second == 1 && section != "" || (it->second != 1 && it->second != 0)) throw Exception("Exception: EquDirective section index is not valid.");
		if (it->second == 1) section = it->first;
	}
	if (literal > 0xFFFFFFFFll || literal < -0x80000000ll) throw Exception("Exception: EquDirective literal value is out of range.");
	symTab->addSymbol(literal, 'O', section == "" ? "*ABS*" : section, symbol);
}

std::string EquDirective::str() {
	std::string result;
	result += "Name: " + symbol + "\n";
	result += "Literal: " + std::to_string(literal) + "\n";
	result += "Add syms:";
	for (int i = 0; i < addSyms.size(); i++) result += " " + addSyms.at(i);
	result += "\nSub syms:";
	for (int i = 0; i < subSyms.size(); i++) result += " " + subSyms.at(i);
	result += "\n";
	return result;
}