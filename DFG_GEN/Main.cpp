#include <string>
#include <iostream>
#include "DFG_Parser.h"

int main() {
	DataFlowGraph	DFG;
	DFG_maker		Parser;
	std::string		names;
	cin >> names;
	Parser.analyze_ll((names+".ll").c_str());
	DFG = Parser.get_DFG();
	DFG.OutputPNG((names+".png").c_str());
	DFG.OutputStatement((names + ".txt").c_str());
	return 0;
}
