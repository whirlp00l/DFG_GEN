#pragma once
// This file describe a class that represent DFG.

#include <vector>
#include <map>
#include <set>
#include <string.h>
using namespace std;


enum InstructionType { STORE, LOAD, ADD, MULT, DIV, SHIFT, MOV, JUMP, CJUMP, NOP, NUL };
#define Number_of_InstructionType 11

class InstructionLatency {
private:
	int store_latency, load_latency;
	int add_latency, mult_latency;
	int div_latency, shift_latency;
	int mov_latency, jump_latency;
	int cjump_latency, nop_latency;
	int colors[11];

public:

	InstructionLatency() {
		store_latency = 1;	load_latency = 1;                 //all execute in 1 cycle
		add_latency = 1;		mult_latency = 1;
		div_latency = 1;		shift_latency = 1;
		mov_latency = 1;		jump_latency = 1;
		cjump_latency = 1;	nop_latency = 1;
		colors[STORE] = 0x0000ff;		colors[LOAD] = 0x00ffff;
		colors[ADD] = 0xffffff;		colors[MULT] = 0x00ff00;
		colors[DIV] = 0xffff00;		colors[SHIFT] = 0xff00ff;
		colors[MOV] = 0x7700ff;		colors[JUMP] = 0x0077ff;
		colors[CJUMP] = 0x000077;		colors[NOP] = 0x777777;
		colors[NUL] = 0x000000;
	}

	void configure(int st, int ld, int add, int mult, int div, int shift, int mov, int jmp, int cjmp, int nop) {
		store_latency = st;		load_latency = ld;
		add_latency = add;		mult_latency = mult;
		div_latency = div;		shift_latency = shift;
		mov_latency = mov;		jump_latency = jmp;
		cjump_latency = cjmp;		nop_latency = nop;
		return;
	}
	int get_latency_of(int inst_type) {
		switch (inst_type) {
		case STORE: return store_latency;
		case LOAD:	return load_latency;
		case ADD:	return add_latency;
		case MULT:	return mult_latency;
		case DIV:	return div_latency;
		case SHIFT:	return shift_latency;
		case MOV:	return mov_latency;
		case JUMP:	return jump_latency;
		case CJUMP:	return cjump_latency;
		case NOP:	return nop_latency;
		};
		return 0;
	}

	int get_color_of(int inst_type) { return colors[inst_type]; }
	char *get_InstTypeName(int inst_type) {
		switch (inst_type) {
		case STORE:		return "STORE";
		case LOAD:		return "LOAD";
		case ADD:		return "ADD";
		case MULT:		return "MULT";
		case DIV:		return "DIV";
		case SHIFT:		return "SHIFT";
		case MOV:		return "MOV";
		case JUMP:		return "JUMP";
		case CJUMP:		return "CJUMP";
		case NOP:		return "NOP";
		};
		return "NULL";
	}

};

class DFG_vertex {
private:
	char name[64];
	int ID, inst_type;
	int num_of_parents, num_of_children;
	int num_of_rec_parents, num_of_rec_children;
	int ASAP, ALAP, MOB, DEPTH, HEIGHT, wd_ALAP;
	std::vector<int> parents, children;
	std::vector<int> parents_distance, children_distance;

public:
	DFG_vertex(char *n, int inst_t, int id) {
		clear();
		strcpy(name, n);
		inst_type = inst_t;
		ID = id;
	}
	DFG_vertex() { clear(); }
	void clear() {
		strcpy(name, "");
		inst_type = NUL;
		parents.clear();
		children.clear();
		parents_distance.clear();
		children_distance.clear();
		num_of_parents = 0;
		num_of_children = 0;
		num_of_rec_parents = 0;
		num_of_rec_children = 0;
		ID = -1;
		ASAP = -1;
		ALAP = -1;
		MOB = -1;
		DEPTH = -1;
		HEIGHT = -1;
		wd_ALAP = -1;
		return;
	}

	void add_children(int children_id, int distance) {
		bool flag = true;
		for (int i = 0; i<this->get_Num_of_Children(); i++) { if (this->get_ChildID(i) == children_id) flag = false; }

		if (flag) {
			children.push_back(children_id);
			children_distance.push_back(distance);
			if (distance != 0) num_of_rec_children++;
			num_of_children++;
		}
		return;
	}
	void add_parents(int parent_id, int distance) {
		bool flag = true;
		for (int i = 0; i<this->get_Num_of_Parents(); i++) { if (this->get_ParentID(i) == parent_id) flag = false; }
		if (flag) {
			parents.push_back(parent_id);
			parents_distance.push_back(distance);
			if (distance != 0) num_of_rec_parents++;
			num_of_parents++;
		}
		return;
	}
	char *get_Name() { return name; }
	char *get_InstTypeName() {
		switch (inst_type) {
		case STORE:		return "STORE";
		case LOAD:		return "LOAD";
		case ADD:		return "ADD";
		case MULT:		return "MULT";
		case DIV:		return "DIV";
		case SHIFT:		return "SHIFT";
		case MOV:		return "MOV";
		case JUMP:		return "JUMP";
		case CJUMP:		return "CJUMP";
		case NOP:		return "NOP";
		};
		return "NULL";
	}
	int get_InstType() { return inst_type; }
	int get_Num_of_Parents() { return num_of_parents; }
	int get_Num_of_Children() { return num_of_children; }
	int get_Num_of_rec_Parents() { return num_of_rec_parents; }
	int get_Num_of_rec_Children() { return num_of_rec_children; }
	int get_ParentID(int i) { return parents[i]; }
	int get_ChildID(int i) { return children[i]; }
	int get_Child_distance(int i) { return children_distance[i]; }
	int get_Parent_distance(int i) { return parents_distance[i]; }
	int get_ID() { return ID; }
	int get_ASAP() { return ASAP; }
	int get_ALAP() { return ALAP; }
	int get_MOB() { return MOB; }
	int get_DEPTH() { return DEPTH; }
	int get_HEIGHT() { return HEIGHT; }

	int get_wd_ALAP() { return wd_ALAP; }

	void set_ASAP(int s) { ASAP = s; }
	void set_ALAP(int s) { ALAP = s; }
	void set_MOB(int s) { MOB = s; }
	void set_DEPTH(int s) { DEPTH = s; }
	void set_HEIGHT(int s) { HEIGHT = s; }
	void set_wd_ALAP(int s) { wd_ALAP = s; }
};

class DataFlowGraph {
private:
public:
	std::map <int, DFG_vertex> Vertices;
	InstructionLatency	InstructionModel;

	DataFlowGraph() { Vertices.clear(); }
	void add_vertex(int id, char *name, int inst_type) {
		DFG_vertex temp(name, inst_type, id);
		if (search_vertex(id) == NULL) Vertices.insert(pair<int, DFG_vertex>(id, temp));
		else {
			printf("ID:%d is already used.\n", id);
			exit(1);
		}
		return;
	}

	DFG_vertex* search_vertex(int id) {
		map<int, DFG_vertex>::iterator p;
		p = Vertices.find(id);
		if (p != Vertices.end()) return &(p->second);
		else return NULL;
	}

	void draw_edge(int parent_id, int child_id, int distance) {
		if (distance == 0) this->draw_edge(parent_id, child_id);
		else {
			DFG_vertex *p, *c;
			p = search_vertex(parent_id);
			c = search_vertex(child_id);
			if (p != NULL && c != NULL) {
				p->add_children(child_id, distance);
				c->add_parents(parent_id, distance);
			}
		}
		return;
	}
	void draw_edge(int parent_id, int child_id) {
		DFG_vertex *p, *c;
		p = search_vertex(parent_id);
		c = search_vertex(child_id);
		if (p != NULL && c != NULL) {
			p->add_children(child_id, 0);
			c->add_parents(parent_id, 0);
		}
		return;
	}

	int  get_number_of_vertices() { return (int)Vertices.size(); }
	int  has_recurrence() {
		map<int, DFG_vertex>::iterator p;
		int num = 0;
		for (p = Vertices.begin(); p != Vertices.end(); p++) {
			for (int i = 0; i<p->second.get_Num_of_Parents(); i++) if (p->second.get_Parent_distance(i) != 0) num++;
		}
		return num;
	}
	void remove_vertex(int id) {
		std::map <int, DFG_vertex>::iterator p = Vertices.find(id);
		Vertices.erase(id);
		return;
	}
	void print_DFG() {
		map<int, DFG_vertex>::iterator p;
		DFG_vertex temp;
		for (p = Vertices.begin(); p != Vertices.end(); p++) print_DFG(p->first);
		return;
	}
	void print_DFG(int ID) {
		map<int, DFG_vertex>::iterator p;
		DFG_vertex temp;
		DFG_vertex *temp_p;

		for (p = Vertices.begin(); p != Vertices.end(); p++) {
			if (p->second.get_ID() == ID) {
				temp = p->second;
				printf("---------------------------------------\n");
				printf("ID:%d  Name:%s, Inst:%s\n", p->first, temp.get_Name(), temp.get_InstTypeName());
				printf("# of Parents%d,  # of Children%d\n", temp.get_Num_of_Parents(), temp.get_Num_of_Children());
				printf("ASAP=%d, ALAP=%d, MOB=%d, DEPTH=%d, HEIGHT=%d\n", temp.get_ASAP(), temp.get_ALAP(), temp.get_MOB(), temp.get_DEPTH(), temp.get_HEIGHT());
				for (int i = 0; i<temp.get_Num_of_Parents(); i++) {
					temp_p = search_vertex(temp.get_ParentID(i));
					printf("Parent No.%d: %s\n", i + 1, temp_p->get_Name());
				}
				for (int i = 0; i<temp.get_Num_of_Children(); i++) {
					temp_p = search_vertex(temp.get_ChildID(i));
					printf("Child No.%d: %s\n", i + 1, temp_p->get_Name());
				}
				printf("---------------------------------------\n");
			}
		}
		return;
	}

	void OutputPNG(const char * filename) {
		FILE *fp;
		map<int, DFG_vertex>::iterator p;
		DFG_vertex temp;
		char cmd_line[100];

		if ((fp = fopen("temp.dot", "w")) == NULL) {
			printf("file open error!!\n");
			exit(1);
		}

		fprintf(fp, "digraph G {\n");
		for (p = Vertices.begin(); p != Vertices.end(); p++) {
			temp = p->second;
			fprintf(fp, "ND%d [label=\"(%d):%s\\n[%s]\"];\n", p->first, temp.get_ID(), temp.get_Name(), temp.get_InstTypeName());
			for (int i = 0; i<temp.get_Num_of_Children(); i++) {
				if (temp.get_Child_distance(i) != 0)fprintf(fp, "ND%d -> ND%d [label=%d, color=red];\n", p->first, temp.get_ChildID(i), InstructionModel.get_latency_of(temp.get_InstType()));
				else fprintf(fp, "ND%d -> ND%d [label=%d];\n", p->first, temp.get_ChildID(i), InstructionModel.get_latency_of(temp.get_InstType()));
			}
		}
		fprintf(fp, "}\n");

		fclose(fp);
		sprintf(cmd_line, "dot -Tpng temp.dot > %s", filename);
		system(cmd_line);
		return;

	}



	void OutputStatement(const char * filename) {
		DFG_vertex *c;
		FILE *fp;

		if ((fp = fopen(filename, "w")) == NULL) {
			printf("file open error!!\n");
			exit(1);
		}
		for (map<int, DFG_vertex>::iterator p = Vertices.begin(); p != Vertices.end(); p++) {
			c = &p->second;
			fprintf(fp, "DFG0.add_vertex(%d,\"%s\",%s);\n", c->get_ID(), c->get_Name(), c->get_InstTypeName());
		}
		for (map<int, DFG_vertex>::iterator p = Vertices.begin(); p != Vertices.end(); p++) {
			c = &p->second;
			for (int i = 0; i<c->get_Num_of_Children(); i++) {
				fprintf(fp, "DFG0.draw_edge(%d,%d,%d);\n", c->get_ID(), c->get_ChildID(i), c->get_Child_distance(i));
			}
		}
		fclose(fp);
		return;
	}
};
