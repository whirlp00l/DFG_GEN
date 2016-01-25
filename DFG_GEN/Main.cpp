#include <string>
#include <iostream>
#include "DFG_Parser.h"
#include "HW.h"
#include "Scheduler.h"

int main() {
	DataFlowGraph	DFG;
	DFG_maker		Parser;
	char names[100];
	scanf("%s", names);
	Parser.analyze_ll(strcat(names,".ll"));
	DFG = Parser.get_DFG();
	DFG.print_DFG();
	//DFG.OutputPNG((names+".png").c_str());
	//DFG.OutputStatement((names + ".txt").c_str());

	HardwareResource HW(2, 2);
	Scheduler ILP(&DFG, &HW);
	ILP.calc_L();
	ILP.PrintLPFile();
	return 0;
}
