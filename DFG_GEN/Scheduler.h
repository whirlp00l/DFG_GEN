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
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l - 1, i, 'N');
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l - 1, i, 'W');
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l - 1, i, 'S');
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l - 1, i, 'E');
						fprintf(fp, " + D(%d,%d,%d,%c)", edge_id, l - 1, i, 'L');
						fprintf(fp, " >= 0\n");
				}
			}

			//routing path constraint
			for (int l = 1; l < l; l++) {
				for (int i = 0; i < HW->num_of_processor; i++) {
					for (int dir = 0; dir < 5; dir++) {
						if (dir == 0 && HW->resources[i].CH.N>0) { //N
							
						
						}
					}
				}
			}
		}
		
		


		fclose(fp);

	}
};