#include <string>
#include <iostream>
#include "DFG_Parser.h"
#include "HW.h"
#include "Scheduler.h"

int main() {
	DataFlowGraph	DFG;
	DFG_maker		Parser;

	DFG.add_vertex(1, "n1", LOAD);
	DFG.add_vertex(2, "n2", LOAD);
	DFG.add_vertex(3, "n3", ADD);
	DFG.add_vertex(4, "n4", MULT);
	DFG.add_vertex(5, "n5", ADD);
	DFG.add_vertex(6, "n6", MULT);
	DFG.draw_edge(1, 2);
	DFG.draw_edge(1, 3);
	DFG.draw_edge(2, 5);
	DFG.draw_edge(4, 5);
	DFG.draw_edge(5, 6);

	//char names[100];
	//scanf("%s", names);
	//Parser.analyze_ll(strcat(names,".ll"));
	//DFG = Parser.get_DFG();
	//DFG.print_DFG();
	//DFG.OutputPNG((names+".png").c_str());
	//DFG.OutputStatement((names + ".txt").c_str());

	HardwareResource HW(2, 2);
	Scheduler ILP(&DFG, &HW);
	ILP.calc_L();
	ILP.PrintLPFile();
	return 0;
}
