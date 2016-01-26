#pragma once
#include <iostream>
#include "DFG.h"
#include "HW.h"
#include <set>
#include <cstdio>

class Scheduler {
private:
	DataFlowGraph *DFG;
	HardwareResource *HW;
	int MAX_WD;
	int L;
public:
	void calc_L() {
		if (L > 0)
			return;
		L = 0;
		for (auto it = DFG->Vertices.begin(); it != DFG->Vertices.end(); it++) {
			L += DFG->InstructionModel.get_latency_of(it->second.get_InstType());
		}
		L += HW->getMaxWD();
	}
	Scheduler(DataFlowGraph* dfg, HardwareResource* hw) : DFG(dfg), HW(hw), MAX_WD(-1), L(-1) {};
	
	void PrintLPFile() {
		FILE *fp;
		if ((fp = fopen("Output_lpfile.lp", "w")) == NULL) {
			printf("Can't open file!\n");
			return;
		}
		std::vector<DFG_vertex*> noChildNodes;
		for (auto p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
			if (p->second.get_Num_of_Children() == 0) {
				printf("Node %d has nochild\n", p->second.get_ID());
				noChildNodes.push_back(&(p->second));
			}
		}
		fprintf(fp, "Minimize\n");
		fprintf(fp, "FINAL\n");
		fprintf(fp, "Subject to\n");
		fprintf(fp, "\\EndNode Constraint\n");
		//X_op_t_p
		for (auto p = noChildNodes.begin(); p != noChildNodes.end(); p++) {
			int operation = (*p)->get_ID();
			bool firstline = true;
			for (int l = 1; l < L; l++) {
				for (int i = 0; i < HW->num_of_processor; i++) {
					if (firstline) {
						fprintf(fp, "  %d X(%d,%d,%d)", l, operation, l, i); // operation binding to time l and processro i;
						firstline = false;
					}
					else
						fprintf(fp, " + %d X(%d,%d,%d)", l, operation, l, i); // operation binding to time l and processro i;
				}
			}
			fprintf(fp, " - FINAL <= 0\n");
		}

		fprintf(fp, "\\Uniqueness Constraint\n");
		//\Sigma_t\Sigma_p X_op_t_p = 1
		for (auto p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
			int operation = p->second.get_ID();
			bool firstline = true;
			for (int l = 1; l < L; l++) {
				for (int i = 0; i < HW->num_of_processor; i++) {
					if (firstline) {
						fprintf(fp, "  X(%d,%d,%d)", operation, l, i); // operation binding to time l and processro i;
						firstline = false;
					}
					else
						fprintf(fp, " + X(%d,%d,%d)", operation, l, i); // operation binding to time l and processro i;
				}
			}
			fprintf(fp, " = 1\n");
		}
		//Data Dependence for X_op_t_p and D_i_t_p_ch;
		int edge_id = 0;
		for (auto p = DFG->Edges.begin(); p != DFG->Edges.end(); p++,edge_id++) {
			int src_id = p->get_Src()->get_ID();
			int dst_id = p->get_Dst()->get_ID();
			bool firstline = true;
			for (int l = 1; l < L; l++) {
				for (int i = 0; i < HW->num_of_processor; i++) {
						fprintf(fp, " - X(%d,%d,%d)", dst_id, l, i); // operation binding to time l and processro i;
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l - 1, i, 'N');
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l - 1, i, 'W');
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l - 1, i, 'S');
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l - 1, i, 'E');
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l - 1, i, 'L');
						fprintf(fp, " >= 0\n");

						fprintf(fp, " - X(%d,%d,%d)", src_id, l, i); // operation binding to time l and processro i;
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, i, 'N');
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, i, 'W');
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, i, 'S');
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, i, 'E');
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, i, 'L');
						fprintf(fp, " >= 0\n");
				}
			}

			//routing path constraint
			for (int l = 1; l < l; l++) {
				for (int i = 0; i < HW->num_of_processor; i++) {
					if (HW->resources[i].CH.N>0) { //N
						int target_processor_id = i - HW->Width;
						if (target_processor_id >= 0) {
							fprintf(fp, " - D(%d,%d,%d)", edge_id, l, i); // operation binding to time l and processro i;
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'N');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'W');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'S');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'E');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'L');
							fprintf(fp, " + X(%d,%d,%d)", dst_id, l + 1, target_processor_id);
							fprintf(fp, " >= 0\n");
						}
					}
					else if (HW->resources[i].CH.W>0) { //W
						int target_processor_id = i - 1;
						if (target_processor_id >= 0) {
							fprintf(fp, " - D(%d,%d,%d,%c)", edge_id, l, i, 'W'); // operation binding to time l and processro i;
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'N');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'W');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'S');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'E');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'L');
							fprintf(fp, " + X(%d,%d,%d)", dst_id, l + 1, target_processor_id);
							fprintf(fp, " >= 0\n");
						}
					}
					else if (HW->resources[i].CH.S>0) { //S
						int target_processor_id = i + HW->Width;
						if (target_processor_id <= HW->num_of_processor) {
							fprintf(fp, " - D(%d,%d,%d,%c)", edge_id, l, i, 'S'); // operation binding to time l and processro i;
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'N');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'W');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'S');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'E');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'L');
							fprintf(fp, " + X(%d,%d,%d)", dst_id, l + 1, target_processor_id);
							fprintf(fp, " >= 0\n");
						}
					}
					else if (HW->resources[i].CH.E>0) { //E
						int target_processor_id = i + 1;
						if (target_processor_id >= 0) {
							fprintf(fp, " - D(%d,%d,%d,%c)", edge_id, l, i, 'E'); // operation binding to time l and processro i;
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'N');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'W');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'S');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'E');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'L');
							fprintf(fp, " + X(%d,%d,%d)", dst_id, l + 1, target_processor_id);
							fprintf(fp, " >= 0\n");
						}
					}
					else if (HW->resources[i].CH.W>0) { //L
						int target_processor_id = i;
						if (target_processor_id >= 0) {
							fprintf(fp, " - D(%d,%d,%d,%c)", edge_id, l, i, 'L'); // operation binding to time l and processro i;
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'N');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'W');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'S');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'E');
							fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l + 1, target_processor_id, 'L');
							fprintf(fp, " + X(%d,%d,%d)", dst_id, l + 1, target_processor_id);
							fprintf(fp, " >= 0\n");
						}
					}	
				}
			}//end data path constraint

			//wire constraint
			for (int l = 1; l < L; l++) {
				for (int i = 0; i < HW->num_of_processor; i++) {
					if (HW->resources[i].CH.E>0) {
						if (i + 1 < HW->num_of_processor) {
							if (HW->resources[i + 1].CH.W > 0) {
								fprintf(fp, "  D(%d,%d,%d,%c)", edge_id, l, i, 'E');
								fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l, i + 1, 'W');
								fprintf(fp, " <= 1\n");
							}
						}
					}
					else if (HW->resources[i].CH.S>0) {
						if (i + HW->Width < HW->num_of_processor) {
							if (HW->resources[i + HW->Width].CH.W>0) {
								fprintf(fp, "  D(%d,%d,%d,%c)", edge_id, l, i, 'S');
								fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l, i + HW->Width, 'N');
								fprintf(fp, " <= 1\n");
							}
						}
					}
				}
			}
		}
//////////TODO///////////////////////////////
		//data exist constraint
		fprintf(fp, "\\data exist constraint");
		for (int l = 1; l < L; l++) {
			for (int i = 0; i < HW->num_of_processor; i++) {
				bool firstline = true;
				for (int edge_id = 0; edge_id < DFG->Edges.size(); edge_id++) {
					if (firstline) {
						fprintf(fp, "  D(%d,%d,%d,%c)", edge_id, l, i, 'N');
						firstline = false;
					}
					else
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l, i, 'N');
				}
				fprintf(fp, " <= 1\n");
				firstline = true;
				for (int edge_id = 0; edge_id < DFG->Edges.size(); edge_id++) {
					if (firstline) {
						fprintf(fp, "  D(%d,%d,%d,%c)", edge_id, l, i, 'W');
						firstline = false;
					}
					else
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l, i, 'W');
				}
				fprintf(fp, " <= 1\n");
				firstline = true;
				for (int edge_id = 0; edge_id < DFG->Edges.size(); edge_id++) {
					if (firstline) {
						fprintf(fp, "  D(%d,%d,%d,%c)", edge_id, l, i, 'S');
						firstline = false;
					}
					else
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l, i, 'S');
				}
				fprintf(fp, " <= 1\n");
				firstline = true;
				for (int edge_id = 0; edge_id < DFG->Edges.size(); edge_id++) {
					if (firstline) {
						fprintf(fp, "  D(%d,%d,%d,%c)", edge_id, l, i, 'E');
						firstline = false;
					}
					else
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l, i, 'E');
				}
				fprintf(fp, " <= 1\n");
				firstline = true;
				for (int edge_id = 0; edge_id < DFG->Edges.size(); edge_id++) {
					if (firstline) {
						fprintf(fp, "  D(%d,%d,%d,%c)", edge_id, l, i, 'L');
						firstline = false;
					}
					else
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l, i, 'L');
				}
				fprintf(fp, " <= 1\n");
			}
		}
		
		fprintf(fp, "Bounds\n");
		//set unavaliable wire/processr to 0
		for (int l = 0; l < L+1; l++) {
			for (int i = 0; i < HW->num_of_processor; i++) {
				for (int edge_id = 0; edge_id < DFG->Edges.size(); edge_id++) {
					if (HW->resources[i].CH.N == 0)
						fprintf(fp, "  D(%d,%d,%d,%c) = 0\n", edge_id, l, i, 'N');
					if (HW->resources[i].CH.W == 0)
						fprintf(fp, "  D(%d,%d,%d,%c) = 0\n", edge_id, l, i, 'W');
					if (HW->resources[i].CH.S == 0)
						fprintf(fp, "  D(%d,%d,%d,%c) = 0\n", edge_id, l, i, 'S');
					if (HW->resources[i].CH.E == 0)
						fprintf(fp, "  D(%d,%d,%d,%c) = 0\n", edge_id, l, i, 'E');
					if (HW->resources[i].CH.L == 0)
						fprintf(fp, "  D(%d,%d,%d,%c) = 0\n", edge_id, l, i, 'L');
				}
			}
		}
		fprintf(fp, "Binary\n");
		//all binary
		for (auto p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
			int operation = p->second.get_ID();
			bool firstline = true;
			for (int l = 1; l < L; l++) {
				for (int i = 0; i < HW->num_of_processor; i++) {
					fprintf(fp, "X(%d,%d,%d)\n", operation, l, i); // operation binding to time l and processro i;
				}
			}
		}

		for (int l = 0; l < L+1; l++) {
			for (int i = 0; i < HW->num_of_processor; i++) {
				for (int edge_id = 0; edge_id < DFG->Edges.size(); edge_id++) {
					if (HW->resources[i].CH.N > 0)
						fprintf(fp, "  D(%d,%d,%d,%c)\n", edge_id, l, i, 'N');
					if (HW->resources[i].CH.W > 0)
						fprintf(fp, "  D(%d,%d,%d,%c)\n", edge_id, l, i, 'W');
					if (HW->resources[i].CH.S > 0)
						fprintf(fp, "  D(%d,%d,%d,%c)\n", edge_id, l, i, 'S');
					if (HW->resources[i].CH.E > 0)
						fprintf(fp, "  D(%d,%d,%d,%c)\n", edge_id, l, i, 'E');
					if (HW->resources[i].CH.L > 0)
						fprintf(fp, "  D(%d,%d,%d,%c)\n", edge_id, l, i, 'L');
				}
			}
		}

		fprintf(fp, "end\n");

		fclose(fp);

	}
};