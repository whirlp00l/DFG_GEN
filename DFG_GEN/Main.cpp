#include <string>
#include <iostream>
#include "DFG_Parser.h"
#include "HW.h"
#include "Scheduler.h"

int main() {
	DataFlowGraph	DFG;
	DFG_maker		Parser;
	
	string mode;
	cin >> mode;
	if (mode == "simple") {
		DFG.add_vertex(1, "n1", LOAD);
		DFG.add_vertex(2, "n2", LOAD);
		DFG.add_vertex(3, "n3", ADD);
		DFG.add_vertex(4, "n4", MULT);
		DFG.add_vertex(5, "n5", ADD);
		//DFG.add_vertex(6, "n6", MULT);
		DFG.draw_edge(1, 2);
		DFG.draw_edge(1, 3);
		DFG.draw_edge(1, 4);
		DFG.draw_edge(1, 5);
		//DFG.draw_edge(5, 6);
	}
	else if(mode == "iir") {
		DFG.add_vertex(0, "LOAD", LOAD);
		DFG.add_vertex(1, "LOAD", LOAD);
		DFG.add_vertex(2, "LOAD", LOAD);
		DFG.add_vertex(3, "MUL", MULT);
		DFG.add_vertex(4, "SUB", ADD);
		DFG.add_vertex(5, "LOAD", LOAD);
		DFG.add_vertex(6, "LOAD", LOAD);
		DFG.add_vertex(7, "MUL", MULT);
		DFG.add_vertex(8, "SUB", ADD);
		DFG.add_vertex(9, "STORE", STORE);
		DFG.add_vertex(10, "LOAD", LOAD);
		DFG.add_vertex(11, "MUL", MULT);
		DFG.add_vertex(12, "ADD", ADD);
		DFG.add_vertex(13, "LOAD", LOAD);
		DFG.add_vertex(14, "MUL", MULT);
		DFG.add_vertex(15, "ADD", ADD);
		DFG.add_vertex(16, "LOAD", LOAD);
		DFG.add_vertex(17, "MUL", MULT);
		DFG.add_vertex(18, "MUL", MULT);
		DFG.add_vertex(19, "ADD", ADD);
		DFG.add_vertex(20, "STORE", STORE);
		DFG.add_vertex(21, "STORE", STORE);
		DFG.add_vertex(22, "SUB", ADD);
		DFG.add_vertex(23, "STORE", STORE);
		DFG.draw_edge(0, 4, 0);
		DFG.draw_edge(1, 3, 0);
		DFG.draw_edge(2, 3, 0);
		DFG.draw_edge(2, 21, 0);
		DFG.draw_edge(3, 4, 0);
		DFG.draw_edge(4, 8, 0);
		DFG.draw_edge(5, 7, 0);
		DFG.draw_edge(5, 12, 0);
		DFG.draw_edge(6, 7, 0);
		DFG.draw_edge(7, 8, 0);
		DFG.draw_edge(7, 9, 0);
		DFG.draw_edge(8, 11, 0);
		DFG.draw_edge(8, 13, 0);
		DFG.draw_edge(8, 22, 0);
		DFG.draw_edge(10, 11, 0);
		DFG.draw_edge(11, 15, 0);
		DFG.draw_edge(12, 14, 0);
		DFG.draw_edge(12, 18, 0);
		DFG.draw_edge(13, 14, 0);
		DFG.draw_edge(14, 15, 0);
		DFG.draw_edge(15, 19, 0);
		DFG.draw_edge(16, 17, 0);
		DFG.draw_edge(17, 18, 0);
		DFG.draw_edge(18, 19, 0);
		DFG.draw_edge(19, 20, 0);
		DFG.draw_edge(22, 23, 0);
	}
	else if(mode == "filter") {
		DFG.add_vertex(0, "PHI", NOP);
		DFG.add_vertex(1, "ADD", ADD);
		DFG.add_vertex(2, "CAST", NOP);
		DFG.add_vertex(4, "ADD", ADD);
		DFG.add_vertex(5, "GETELEMENTPTR", ADD);
		DFG.add_vertex(6, "LOAD", LOAD);
		DFG.add_vertex(8, "LOAD", LOAD);
		DFG.add_vertex(9, "SHL", SHIFT);
		DFG.add_vertex(10, "ADD", ADD);
		DFG.add_vertex(11, "GETELEMENTPTR", ADD);
		DFG.add_vertex(12, "LOAD", LOAD);
		DFG.add_vertex(13, "ADD", ADD);
		DFG.add_vertex(14, "SUB", ADD);
		DFG.add_vertex(15, "STORE", STORE);
		DFG.add_vertex(16, "SETEQ", MOV);
		DFG.draw_edge(0, 1, 0);
		DFG.draw_edge(1, 2, 0);
		DFG.draw_edge(1, 3, 0);
		DFG.draw_edge(1, 7, 0);
		DFG.draw_edge(1, 16, 0);
		DFG.draw_edge(2, 4, 0);
		DFG.draw_edge(2, 10, 0);
		DFG.draw_edge(3, 15, 0);
		DFG.draw_edge(4, 5, 0);
		DFG.draw_edge(5, 6, 0);
		DFG.draw_edge(6, 13, 0);
		DFG.draw_edge(7, 8, 0);
		DFG.draw_edge(8, 9, 0);
		DFG.draw_edge(9, 14, 0);
		DFG.draw_edge(10, 11, 0);
		DFG.draw_edge(11, 12, 0);
		DFG.draw_edge(12, 13, 0);
		DFG.draw_edge(13, 14, 0);
		DFG.draw_edge(14, 15, 0);
	}
	else {
		Parser.analyze_ll((mode+".ll").c_str());
		DFG = Parser.get_DFG();
		DFG.print_DFG();
	}
	DFG.OutputPNG((mode+".png").c_str());
	DFG.OutputStatement((mode + ".txt").c_str());

	HardwareResource HW(2, 2);
	Scheduler ILP(&DFG, &HW);
	ILP.calc_L();
	//ILP.set_L(8);
	ILP.PrintLPFile(mode);
	return 0;
}
