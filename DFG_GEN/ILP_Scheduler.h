#pragma once
#include <math.h>
#include <cassert>

#include "DFG.h"
#include "HW.h"

enum SearchDirection { BOTTOM_UP, TOP_DOWN };
enum ScheduleSituation { Nothing, OnlyPred, OnlySucc, Both };
//#define grids 3
//#define MaxTry 3
//#define MaxDistance 3

class SwingModuloScheduler {
private:
	DataFlowGraph *DFG;
	HardwareResource *HardWare;
	InstructionLatency *InstructionModel;
	float ResMII, RecMII;
	int MII, II;
	int InstructionNumber[Number_of_InstructionType];
	int havebothPredSucc, MaxWD, Comp_max_CS;
	//std::vector<DFG_vertex*> ScheduleOrder;

	//std::vector<DFG_vertex*> ScheduleOrderNew;

	// int MinL;

	set<DFG_vertex*> SwingModuloScheduler::Pred_L(set<DFG_vertex*> O) {
		set<DFG_vertex*> ret;
		set<DFG_vertex*>::iterator p;

		ret.clear();
		for (p = O.begin(); p != O.end(); p++) {
			for (int i = 0; i<(*p)->get_Num_of_Parents(); i++) {
				DFG_vertex *current_p;
				current_p = DFG->search_vertex((*p)->get_ParentID(i));
				if (O.find(current_p) == O.end()) ret.insert(current_p);
			}
		}
		return ret;
	}
	set<DFG_vertex*> SwingModuloScheduler::Succ_L(set<DFG_vertex*> O) {
		set<DFG_vertex*> ret;
		set<DFG_vertex*>::iterator p;

		ret.clear();
		for (p = O.begin(); p != O.end(); p++) {
			for (int i = 0; i<(*p)->get_Num_of_Children(); i++) {
				DFG_vertex *current_c;
				current_c = DFG->search_vertex((*p)->get_ChildID(i));
				if (O.find(current_c) == O.end()) ret.insert(current_c);
			}
		}
		return ret;
	}
	set<DFG_vertex*> SwingModuloScheduler::Cap(set<DFG_vertex*> A, set<DFG_vertex*> B) {
		set<DFG_vertex*> res;
		set<DFG_vertex*>::iterator p;
		res.clear();
		for (p = A.begin(); p != A.end(); p++) {
			if (B.find(*p) != B.end()) res.insert(*p);
		}
		return res;
	}
	set<DFG_vertex*> SwingModuloScheduler::Cup(set<DFG_vertex*> A, set<DFG_vertex*> B) {
		set<DFG_vertex*> res;
		set<DFG_vertex*>::iterator p;
		res.clear();
		for (p = A.begin(); p != A.end(); p++) { res.insert(*p); }
		for (p = B.begin(); p != B.end(); p++) { res.insert(*p); }
		return res;
	}

	set<DFG_vertex*> SwingModuloScheduler::Succ(DFG_vertex* v) {
		set<DFG_vertex*> res;
		res.clear();
		for (int i = 0; i<v->get_Num_of_Children(); i++) {
			res.insert(DFG->search_vertex(v->get_ChildID(i)));
		}
		return res;
	}
	set<DFG_vertex*> SwingModuloScheduler::Pred(DFG_vertex* v) {
		set<DFG_vertex*> res;
		res.clear();
		for (int i = 0; i<v->get_Num_of_Parents(); i++) {
			res.insert(DFG->search_vertex(v->get_ParentID(i)));
		}
		return res;
	}
	int  SwingModuloScheduler::calc_latency(int Pid, int Cid) {
		DFG_vertex *parent, *child;
		set<DFG_vertex*> succ;
		set<int> latencies;
		parent = DFG->search_vertex(Pid);
		child = DFG->search_vertex(Cid);
		succ = Succ(parent);
		latencies.clear();

		if (succ.find(child) == succ.end()) {
			for (int i = 0; i<parent->get_Num_of_Children(); i++) {
				if (parent->get_Child_distance(i) == 0) {
					latencies.insert(calc_latency(parent->get_ChildID(i), Cid));
				}
			}
			set<int>::iterator p = latencies.end();
			p--;
			return InstructionModel->get_latency_of(parent->get_InstType()) + (*p);
		}
		else {
			return InstructionModel->get_latency_of(parent->get_InstType()) + InstructionModel->get_latency_of(child->get_InstType());
		}
	}
	int  SwingModuloScheduler::calc_RecMII() {
		std::vector<int> RecMII;
		std::map<int, DFG_vertex>::iterator p;
		RecMII.clear();

		for (p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
			DFG_vertex *current_vertex;
			current_vertex = &(*p).second;
			if (current_vertex->get_Num_of_rec_Children()>0) {
				int destiny = 0;
				for (int i = 0; i<current_vertex->get_Num_of_Children(); i++) {
					if (current_vertex->get_Child_distance(i)>0) {
						int departure = current_vertex->get_ID();
						int distance = current_vertex->get_Child_distance(i);
						destiny = current_vertex->get_ChildID(i);
						float latency = (float)calc_latency(destiny, departure);
						RecMII.push_back((int)((latency / distance) + 0.5));
						// RecMII.push_back((float)(latency/distance));
					}
				}
			}
		}
		int max_RecMII = 0;
		for (int i = 0; i<(int)RecMII.size(); i++) {
			if (RecMII[i]>max_RecMII) max_RecMII = RecMII[i];
		}
		return max_RecMII;
	}
	void SwingModuloScheduler::calc_ASAP(DFG_vertex *v) {
		DFG_vertex *current_parent;

		if (v->get_Num_of_Parents() == v->get_Num_of_rec_Parents()) { v->set_ASAP(1); return; }
		else {
			bool all_parents_calc_ASAP = true;
			int  un_calced_parent;

			for (int i = 0; i<v->get_Num_of_Parents(); i++) {
				current_parent = DFG->search_vertex(v->get_ParentID(i));
				if (current_parent->get_ASAP() < 0 && v->get_Parent_distance(i) == 0) { all_parents_calc_ASAP = false; un_calced_parent = v->get_ParentID(i); }
			}
			if (all_parents_calc_ASAP) {
				int ASAP = -1, p_asap;
				for (int i = 0; i<v->get_Num_of_Parents(); i++) {
					current_parent = DFG->search_vertex(v->get_ParentID(i));
					p_asap = current_parent->get_ASAP() + InstructionModel->get_latency_of(current_parent->get_InstType());
					if (v->get_Parent_distance(i) != 0) p_asap -= MII*v->get_Parent_distance(i);
					if (current_parent->get_ASAP()<0) p_asap = -1;
					if (ASAP<p_asap) ASAP = p_asap;
				}
				v->set_ASAP(ASAP);
				return;
			}
			else {
				calc_ASAP(DFG->search_vertex(un_calced_parent));
				calc_ASAP(v);
			}
		}
		return;
	}
	void SwingModuloScheduler::calc_ALAP(DFG_vertex *v) {
		DFG_vertex *current_child;
		if (v->get_ALAP() >= 0) return;

		if (v->get_Num_of_Children() == v->get_Num_of_rec_Children()) {
			std::map<int, DFG_vertex>::iterator p;
			int ALAP = 0;
			for (p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) { if (ALAP< p->second.get_ASAP() && p->second.get_ASAP() >= 0)ALAP = p->second.get_ASAP(); }
			v->set_ALAP(ALAP);
			return;
		}
		else {
			bool all_children_calc_ALAP = true;
			int  un_calced_child;

			for (int i = 0; i<v->get_Num_of_Children(); i++) {
				current_child = DFG->search_vertex(v->get_ChildID(i));
				if (current_child->get_ALAP() < 0 && v->get_Child_distance(i) == 0) { all_children_calc_ALAP = false; un_calced_child = v->get_ChildID(i); }
			}
			if (all_children_calc_ALAP) {
				int ALAP = 0x7fffffff;
				int c_alap;
				for (int i = 0; i<v->get_Num_of_Children(); i++) {
					current_child = DFG->search_vertex(v->get_ChildID(i));
					c_alap = current_child->get_ALAP() - InstructionModel->get_latency_of(v->get_InstType());
					if (v->get_Child_distance(i) != 0) c_alap += MII*v->get_Child_distance(i);
					if (current_child->get_ALAP()<0) c_alap = 0x7fffffff;
					if (ALAP>c_alap && c_alap >= 0) ALAP = c_alap;
				}
				v->set_ALAP(ALAP);
				return;
			}
			else {
				calc_ALAP(DFG->search_vertex(un_calced_child));
				calc_ALAP(v);
			}
		}
		return;
	}


	void SwingModuloScheduler::calc_wd_ALAP(DFG_vertex *v) {

		DFG_vertex *current_child;
		// int MaxWD=0,
		if (v->get_wd_ALAP() >= 0) return;

		if (v->get_Num_of_Children() == v->get_Num_of_rec_Children()) {
			std::map<int, DFG_vertex>::iterator p;
			int ALAP = 0, ASAP = 0, fu = 0, The1stfu = 0, wd1;
			//Comp_max_CS;
			for (p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
				if (ASAP< p->second.get_ASAP() && p->second.get_ASAP() >= 0)
				{
					ALAP = p->second.get_ALAP();
					ASAP = p->second.get_ASAP();
				}
			}
			// compute the max wire delay
			for (fu = 1; fu <= HardWare->get_number_of_PU(); fu++) {
				wd1 = HardWare->calc_wiredelay(The1stfu, fu);
				if (MaxWD<wd1) MaxWD = wd1;
			}
			Comp_max_CS = ALAP + (ASAP - 1)*MaxWD;
			v->set_wd_ALAP(Comp_max_CS);
			//v->set_ALAP(ALAP);
			return;
		}
		else {
			bool all_children_calc_wd_ALAP = true;
			int  un_calced_child;
			for (int i = 0; i<v->get_Num_of_Children(); i++) {
				current_child = DFG->search_vertex(v->get_ChildID(i));
				if (current_child->get_wd_ALAP() < 0 && v->get_Child_distance(i) == 0) { all_children_calc_wd_ALAP = false; un_calced_child = v->get_ChildID(i); }
			}
			if (all_children_calc_wd_ALAP) {
				int ALAP2 = 0x7fffffff;
				int c_alap;

				if (v->get_MOB() != 0) {
					for (int i = 0; i<v->get_Num_of_Children(); i++) {
						current_child = DFG->search_vertex(v->get_ChildID(i));
						c_alap = current_child->get_wd_ALAP() - InstructionModel->get_latency_of(v->get_InstType());
						if (v->get_Child_distance(i) != 0) c_alap += MII*v->get_Child_distance(i);
						if (current_child->get_wd_ALAP()<0) c_alap = 0x7fffffff;
						if (ALAP2>c_alap && c_alap >= 0) ALAP2 = c_alap;
					}
					v->set_wd_ALAP(ALAP2);
					return;
				}
				else {
					int wd1ALAP = 0;
					wd1ALAP = v->get_ALAP() + (v->get_ASAP() - 1)*MaxWD;
					v->set_wd_ALAP(wd1ALAP);
				}
			}
			else {
				calc_wd_ALAP(DFG->search_vertex(un_calced_child));
				calc_wd_ALAP(v);
			}
		}
		return;
	}


	void SwingModuloScheduler::calc_MOB(DFG_vertex *v) {
		// v->set_MOB(v->get_ALAP()-v->get_ASAP());
		v->set_MOB(v->get_ALAP() - v->get_ASAP());
		return;
	}



	void SwingModuloScheduler::calc_DEPTH(DFG_vertex *v) {
		DFG_vertex *current_parent;

		if (v->get_Num_of_Parents() == v->get_Num_of_rec_Parents()) { v->set_DEPTH(0); return; }
		else {
			bool all_parents_calc_DEPTH = true;
			int  un_calced_parent;

			for (int i = 0; i<v->get_Num_of_Parents(); i++) {
				current_parent = DFG->search_vertex(v->get_ParentID(i));
				if (current_parent->get_DEPTH() < 0 && v->get_Parent_distance(i) == 0) { all_parents_calc_DEPTH = false; un_calced_parent = v->get_ParentID(i); }
			}
			if (all_parents_calc_DEPTH) {
				int DEPTH = -1, p_depth;
				for (int i = 0; i<v->get_Num_of_Parents(); i++) {
					current_parent = DFG->search_vertex(v->get_ParentID(i));
					p_depth = current_parent->get_DEPTH() + InstructionModel->get_latency_of(current_parent->get_InstType());
					if (current_parent->get_DEPTH()<0) p_depth = -1;
					if (DEPTH<p_depth) DEPTH = p_depth;
				}
				v->set_DEPTH(DEPTH);
				return;
			}
			else {
				calc_DEPTH(DFG->search_vertex(un_calced_parent));
				calc_DEPTH(v);
			}
		}
		return;
	}
	void SwingModuloScheduler::calc_HEIGHT(DFG_vertex *v) {
		DFG_vertex *current_child;
		if (v->get_HEIGHT() >= 0) return;

		if (v->get_Num_of_Children() == v->get_Num_of_rec_Children()) {
			v->set_HEIGHT(0);
			return;
		}
		else {
			bool all_children_calc_HEIGHT = true;
			int  un_calced_child;

			for (int i = 0; i<v->get_Num_of_Children(); i++) {
				current_child = DFG->search_vertex(v->get_ChildID(i));
				if (current_child->get_HEIGHT() < 0 && v->get_Child_distance(i) == 0) { all_children_calc_HEIGHT = false; un_calced_child = v->get_ChildID(i); }
			}
			if (all_children_calc_HEIGHT) {
				int HEIGHT = -1;
				int c_height;
				for (int i = 0; i<v->get_Num_of_Children(); i++) {
					current_child = DFG->search_vertex(v->get_ChildID(i));
					c_height = current_child->get_HEIGHT() + InstructionModel->get_latency_of(v->get_InstType());
					if (current_child->get_HEIGHT()<0) c_height = -1;
					if (HEIGHT<c_height) HEIGHT = c_height;
				}
				v->set_HEIGHT(HEIGHT);
				return;
			}
			else {
				calc_HEIGHT(DFG->search_vertex(un_calced_child));
				calc_HEIGHT(v);
			}
		}
		return;
	}


	void SwingModuloScheduler::add_visitable_vertices(set<DFG_vertex*> *v, DFG_vertex *current, int direction) {
		if (direction == BOTTOM_UP) {
			if (current->get_Num_of_Parents() == current->get_Num_of_rec_Parents()) return;
			else {
				for (int i = 0; i<current->get_Num_of_Parents(); i++) {
					if (current->get_Parent_distance(i) == 0) {
						v->insert(DFG->search_vertex(current->get_ParentID(i)));
						add_visitable_vertices(v, DFG->search_vertex(current->get_ParentID(i)), direction);
					}
				}
			}
		}
		else {
			if (current->get_Num_of_Children() == current->get_Num_of_rec_Children()) return;
			else {
				for (int i = 0; i<current->get_Num_of_Children(); i++) {
					if (current->get_Child_distance(i) == 0) {
						v->insert(DFG->search_vertex(current->get_ChildID(i)));
						add_visitable_vertices(v, DFG->search_vertex(current->get_ChildID(i)), direction);
					}
				}
			}
		}
		return;
	}
	set<DFG_vertex*> SwingModuloScheduler::gen_subgraph(set<DFG_vertex*> *v, int Pid, int Cid) {
		set<DFG_vertex*> res;
		set<DFG_vertex*> parents, children;
		set<DFG_vertex*>::iterator p;
		children.clear(); parents.clear(); res.clear();

		add_visitable_vertices(&parents, DFG->search_vertex(Pid), BOTTOM_UP);
		add_visitable_vertices(&children, DFG->search_vertex(Cid), TOP_DOWN);
		res = Cap(parents, children);
		res = Cap(res, *v);
		res.insert(DFG->search_vertex(Pid));
		res.insert(DFG->search_vertex(Cid));
		for (p = res.begin(); p != res.end(); p++) v->erase(*p);

		return res;

	}
	std::vector<DFG_vertex*> SwingModuloScheduler::partial_ordering(set<DFG_vertex*> v, int direction) { //This function is called in Non-Recurrent ordering Mode 
		std::vector<DFG_vertex*> res;
		std::vector<DFG_vertex*> remain;
		res.clear();
		if (v.size() == 0) return res;

		if (direction == BOTTOM_UP) {
			set<DFG_vertex*>::iterator q;
			int depth = -1, mobility = -1;
			DFG_vertex *current_vertex;

			for (q = v.begin(); q != v.end(); q++) {
				if ((*q)->get_DEPTH() > depth) { current_vertex = *q; depth = (*q)->get_DEPTH(); mobility = (*q)->get_MOB(); }
				else if ((*q)->get_DEPTH() == depth) {
					if ((*q)->get_MOB() < mobility) { current_vertex = *q; depth = (*q)->get_DEPTH(); mobility = (*q)->get_MOB(); }
				}
			}
			v.erase(current_vertex);
			res.push_back(current_vertex);
			remain = partial_ordering(v, direction);
			res.insert(res.end(), remain.begin(), remain.end());
			return res;

		}
		else {
			set<DFG_vertex*>::iterator q;
			int height = -1, mobility = -1;
			DFG_vertex *current_vertex;

			for (q = v.begin(); q != v.end(); q++) {
				if ((*q)->get_HEIGHT() > height) { current_vertex = *q; height = (*q)->get_HEIGHT(); mobility = (*q)->get_MOB(); }
				else if ((*q)->get_HEIGHT() == height) {
					if ((*q)->get_MOB() < mobility) { current_vertex = *q; height = (*q)->get_HEIGHT(); mobility = (*q)->get_MOB(); }
				}
			}
			v.erase(current_vertex);
			res.push_back(current_vertex);
			remain = partial_ordering(v, direction);
			res.insert(res.end(), remain.begin(), remain.end());
			return res;
		}
		return res;
	}
	void SwingModuloScheduler::clear() {
		DFG = NULL;
		HardWare = NULL;
		InstructionModel = NULL;
		RecMII = 0;	ResMII = 0;
		MII = 0;		II = 0;
		MaxWD = 0;
		Comp_max_CS = 0;
		//    ScheduleOrder.clear();
		//ScheduleOrderNew.clear();
		// MinL=0;
		havebothPredSucc = 0;
		for (int i = 0; i<Number_of_InstructionType; i++) InstructionNumber[i] = 0;
		return;
	}

public:
	SwingModuloScheduler::SwingModuloScheduler() { clear(); }
	SwingModuloScheduler::SwingModuloScheduler(DataFlowGraph *d, HardwareResource *h) { clear(); SetResources(d, h); II = MII = 1; }
	void SwingModuloScheduler::SetResources(DataFlowGraph *d, HardwareResource *h) {
		DFG = d;
		HardWare = h;
		InstructionModel = &(DFG->InstructionModel);
		return;
	}

	void SwingModuloScheduler::Analyze_DFG() {
		std::map<int, DFG_vertex>::iterator p;
		DFG_vertex* current_vertex;
		//calculate ResMII;
		int num_of_op = DFG->get_number_of_vertices();
		printf("\n num of vertices is %d\n", num_of_op);
		int num_of_fu = HardWare->get_number_of_PU();
		//    printf("\n num of FU is %d\n",num_of_fu);
		ResMII = (float)ceilf((((float)num_of_op) / ((float)num_of_fu)));
		RecMII = (float)calc_RecMII();
		if (ResMII>RecMII) MII = (int)(ResMII + 0.999999999999);
		else if (ResMII<RecMII) MII = (int)(RecMII + 0.999999999999);
		else MII = (int)(ResMII + 0.999999999999);
		printf("\nResMII=%f, RecMII=%f, MII=%d\n\n", ResMII, RecMII, MII);
		II = MII;

		for (p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
			current_vertex = &p->second;
			if (current_vertex->get_ASAP()<0) calc_ASAP(current_vertex);
		}
		for (p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
			current_vertex = &p->second;
			if (current_vertex->get_ALAP()<0) calc_ALAP(current_vertex);
		}
		for (p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
			current_vertex = &p->second;
			if (current_vertex->get_MOB()<0) calc_MOB(current_vertex);
		}
		for (p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
			current_vertex = &p->second;
			if (current_vertex->get_DEPTH()<0) calc_DEPTH(current_vertex);
		}
		for (p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
			current_vertex = &p->second;
			if (current_vertex->get_HEIGHT()<0) calc_HEIGHT(current_vertex);
		}


		for (p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
			current_vertex = &p->second;
			if (current_vertex->get_wd_ALAP()<0) calc_wd_ALAP(current_vertex);
		}


		for (p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
			current_vertex = &p->second;
			printf("ID:%02d Name:%s ASAP:%d ALAP:%d MOB:%d DEPTH:%d HEIGHT:%d wd_ALAP: %d\n",
				current_vertex->get_ID(), current_vertex->get_Name(), current_vertex->get_ASAP(),
				current_vertex->get_ALAP(), current_vertex->get_MOB(), current_vertex->get_DEPTH(),
				current_vertex->get_HEIGHT(), current_vertex->get_wd_ALAP());
		}
		printf("This DFG has %d Reccurent Edge.\n", DFG->has_recurrence());

	}



	void SwingModuloScheduler::PrintLPfile() {
		//int num_of_nodes=0;
		int M_loc_x = 0, M_loc_y = 0;
		std::map<int, DFG_vertex>::iterator p;
		std::vector<DFG_vertex*> WithoutChildNodes;
		std::vector<DFG_vertex*>::iterator ver;
		WithoutChildNodes.clear();

		FILE *fp;
		if ((fp = fopen("Output_lpfile.lp", "w")) == NULL) {
			printf("Can't open file!\n");
			return;
		}

		//num_of_nodes=(int)DFG->Vertices.size();
		for (int i = 0; i<HardWare->get_number_of_PU(); i++) {
			if (M_loc_x<HardWare->resources[i].get_x())
				M_loc_x = HardWare->resources[i].get_x();
			if (M_loc_y<HardWare->resources[i].get_y())
				M_loc_y = HardWare->resources[i].get_y();
		}
		//  int Z[num_of_nodes][MaxWD][M_loc_x][M_loc_y];

		printf("MaxWD=%d\n", MaxWD);
		printf("Comp_max_CS=%d\n", Comp_max_CS);

		for (p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
			printf("Node %d has wd_ALAP %d:\n", p->second.get_ID(), p->second.get_wd_ALAP());
		}

		int endNode = 0;
		for (p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
			//DFG_vertex *current_vertex;
			//current_vertex=&(*p).second;
			//if(current_vertex->get_wd_ALAP()==MaxWD)
			if (p->second.get_wd_ALAP() == Comp_max_CS) {
				printf("Nodes %d no child:\n", p->first);
				WithoutChildNodes.push_back(&p->second);
				endNode++;
			}
		}
		printf("the Num of endNode = %d\n", endNode);
		//Object function
		if (WithoutChildNodes.empty()) { printf("withoutchildNodes.empty=true\n"); }
		fprintf(fp, "Minimize\n");
		bool whether1stline1 = true;
		if (!WithoutChildNodes.empty()) {

			if (WithoutChildNodes.size() == 1) {
				// int obj=0;
				printf("Only one endNode!\n");
				ver = WithoutChildNodes.begin();
				int Node_ID = (*ver)->get_ID();
				for (int i = (*ver)->get_ASAP(); i <= (*ver)->get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							//obj+=i*Z[Node_ID][i][j][k];
							printf("M_loc_x=%d\n", M_loc_x);
							printf("M_loc_y=%d\n", M_loc_y);
							if (whether1stline1)
							{
								fprintf(fp, " %d Z(%d,%d,%d,%d)", i, Node_ID, i, j, k);
								whether1stline1 = false;
							}
							else
								fprintf(fp, " + %d Z(%d,%d,%d,%d)", i, Node_ID, i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
			}

			if (WithoutChildNodes.size() == 2) {
				printf("two endNodes!\n");
				DFG_vertex *v1, *v2, *v3;
				//v1=*(WithoutChildNodes.begin());
				bool equal_MOB = false;
				v1 = *(WithoutChildNodes.begin());
				v2 = *(WithoutChildNodes.begin() + 1);
				if (v1->get_MOB()<v2->get_MOB()) {
					//v1=*(WithoutChildNodes.begin());
					//v2=*(WithoutChildNodes.begin()+1);
					equal_MOB = false;
					printf("The endNode %d with smaller Mob:\n", v1->get_ID());
					printf("The endNode %d with larger Mob:\n", v2->get_ID());
				}

				else if (v1->get_MOB()>v2->get_MOB()) {
					printf("The endNode %d with larger Mob\n", v1->get_ID());
					printf("The endNode %d with smaller Mob\n", v2->get_ID());
					v2 = *(WithoutChildNodes.begin());
					v1 = *(WithoutChildNodes.begin() + 1);
					equal_MOB = false;
				}
				else {
					v1 = *(WithoutChildNodes.begin());
					v2 = *(WithoutChildNodes.begin() + 1);
					equal_MOB = true;
					printf("The endNodes %d %d have same Mob:\n", v1->get_ID(), v2->get_ID());
				}

				int n1 = v1->get_ID();
				int n2 = v2->get_ID();
				bool whether1stline2 = true;
				if (!equal_MOB) {
					for (int i = v1->get_ASAP(); i <= v1->get_wd_ALAP(); i++) {
						for (int j = 0; j <= M_loc_x; j++) {
							for (int k = 0; k <= M_loc_y; k++) {
								if (whether1stline2)
								{
									fprintf(fp, " %d Z(%d,%d,%d,%d)", 10 * i, n1, i, j, k);
									whether1stline2 = false;
								}
								else
									fprintf(fp, " + %d Z(%d,%d,%d,%d)", 10 * i, n1, i, j, k);
							}
						}
						fprintf(fp, "\n");
					}
					for (int i = v2->get_ASAP(); i <= v2->get_wd_ALAP(); i++) {
						for (int j = 0; j <= M_loc_x; j++) {
							for (int k = 0; k <= M_loc_y; k++) {
								fprintf(fp, " + %d Z(%d,%d,%d,%d)", i, n2, i, j, k);
							}
						}
						fprintf(fp, "\n");

					}
				}

				else {
					for (int i = v1->get_ASAP(); i <= v1->get_wd_ALAP(); i++) {
						for (int j = 0; j <= M_loc_x; j++) {
							for (int k = 0; k <= M_loc_y; k++) {
								if (whether1stline2)
								{
									fprintf(fp, "  %d Z(%d,%d,%d,%d)", i, n1, i, j, k);
									whether1stline2 = false;
								}
								else
									fprintf(fp, " + %d Z(%d,%d,%d,%d)", i, n1, i, j, k);
							}
						}
						fprintf(fp, "\n");
					}
					for (int i = v2->get_ASAP(); i <= v2->get_wd_ALAP(); i++) {
						for (int j = 0; j <= M_loc_x; j++) {
							for (int k = 0; k <= M_loc_y; k++) {
								fprintf(fp, " + %d Z(%d,%d,%d,%d)", i, n2, i, j, k);
							}
						}
						fprintf(fp, "\n");
					}
				}
			}
		}


		fprintf(fp, "Subject to\n");
		fprintf(fp, "\\Uniqueness constraint:\n");
		int uniq = 0;
		for (p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
			DFG_vertex *cur_vertex;
			cur_vertex = &(*p).second;
			fprintf(fp, "U%d:", uniq);
			uniq++;
			bool whether1stline3 = true;
			for (int i = cur_vertex->get_ASAP(); i <= cur_vertex->get_wd_ALAP(); i++) {
				for (int j = 0; j <= M_loc_x; j++) {
					for (int k = 0; k <= M_loc_y; k++) {
						if (whether1stline3)
						{
							fprintf(fp, " Z(%d,%d,%d,%d)", cur_vertex->get_ID(), i, j, k);
							whether1stline3 = false;
						}
						else
							fprintf(fp, " + Z(%d,%d,%d,%d)", cur_vertex->get_ID(), i, j, k);
					}
				}
				fprintf(fp, "\n");
			}
			fprintf(fp, "= 1\n");
		}

		fprintf(fp, "\\Data dependence constraint:\n");
		for (p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
			for (int child = 0; child<p->second.get_Num_of_Children(); child++) {
				DFG_vertex *cur_child;
				cur_child = DFG->search_vertex(p->second.get_ChildID(child));

				fprintf(fp, "\\dd_%d_%d_1:\n ", p->second.get_ID(), cur_child->get_ID());
				bool whether1stline4 = true;
				//DATA DEPENDENCE: -|a-b|-|c-d|: 
				//(1),-(a-b)-(c-d)= -a + b - c + d 
				//(2),(a-b)-(c-d)= a - b - c + d
				//(3), -(a-b)+(c-d) = -a + b + c - d
				//(4),(a-b)+(c-d) = a -b + c - d
				//case 1	 
				for (int i = cur_child->get_ASAP(); i <= cur_child->get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							if (whether1stline4)
							{
								fprintf(fp, " %d Z(%d,%d,%d,%d)", i, cur_child->get_ID(), i, j, k);
								whether1stline4 = false;
							}
							else
								fprintf(fp, " + %d Z(%d,%d,%d,%d)", i, cur_child->get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				for (int i = p->second.get_ASAP(); i <= p->second.get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " - %d Z(%d,%d,%d,%d)", i, p->second.get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}

				//(1),-(a-b)-(c-d)= -a + b - c + d 
				for (int i = cur_child->get_ASAP(); i <= cur_child->get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " - %d Z(%d,%d,%d,%d)", j, cur_child->get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				for (int i = p->second.get_ASAP(); i <= p->second.get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " + %d Z(%d,%d,%d,%d)", j, p->second.get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				for (int i = cur_child->get_ASAP(); i <= cur_child->get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " - %d Z(%d,%d,%d,%d)", k, cur_child->get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				for (int i = p->second.get_ASAP(); i <= p->second.get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " + %d Z(%d,%d,%d,%d)", k, p->second.get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				fprintf(fp, ">=1\n");
				//case 1 finish


				//case 2
				fprintf(fp, "\\dd_%d_%d_2:\n ", p->second.get_ID(), cur_child->get_ID());
				bool whether1stline5 = true;

				for (int i = cur_child->get_ASAP(); i <= cur_child->get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							if (whether1stline5)
							{
								fprintf(fp, "  %d Z(%d,%d,%d,%d)", i, cur_child->get_ID(), i, j, k);
								whether1stline5 = false;
							}
							else
								fprintf(fp, " + %d Z(%d,%d,%d,%d)", i, cur_child->get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				for (int i = p->second.get_ASAP(); i <= p->second.get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " - %d Z(%d,%d,%d,%d)", i, p->second.get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}

				//(2),(a-b)-(c-d)= a - b - c + d
				for (int i = cur_child->get_ASAP(); i <= cur_child->get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " + %d Z(%d,%d,%d,%d)", j, cur_child->get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				for (int i = p->second.get_ASAP(); i <= p->second.get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " - %d Z(%d,%d,%d,%d)", j, p->second.get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				for (int i = cur_child->get_ASAP(); i <= cur_child->get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " - %d Z(%d,%d,%d,%d)", k, cur_child->get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				for (int i = p->second.get_ASAP(); i <= p->second.get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " + %d Z(%d,%d,%d,%d)", k, p->second.get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				fprintf(fp, ">=1\n");
				// fprintf(fp,"%s\n",obj);


				fprintf(fp, "\\dd_%d_%d_3:\n ", p->second.get_ID(), cur_child->get_ID());
				bool whether1stline6 = true;
				//case 3
				for (int i = cur_child->get_ASAP(); i <= cur_child->get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							if (whether1stline6)
							{
								fprintf(fp, " %d Z(%d,%d,%d,%d)", i, cur_child->get_ID(), i, j, k);
								whether1stline6 = false;
							}
							else
								fprintf(fp, " + %d Z(%d,%d,%d,%d)", i, cur_child->get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				for (int i = p->second.get_ASAP(); i <= p->second.get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " - %d Z(%d,%d,%d,%d)", i, p->second.get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}

				//case 3
				//(3), -(a-b)+(c-d) = -a + b + c - d
				for (int i = cur_child->get_ASAP(); i <= cur_child->get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " - %d Z(%d,%d,%d,%d)", j, cur_child->get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				for (int i = p->second.get_ASAP(); i <= p->second.get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " + %d Z(%d,%d,%d,%d)", j, p->second.get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				for (int i = cur_child->get_ASAP(); i <= cur_child->get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " + %d Z(%d,%d,%d,%d)", k, cur_child->get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				for (int i = p->second.get_ASAP(); i <= p->second.get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " - %d Z(%d,%d,%d,%d)", k, p->second.get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				fprintf(fp, ">=1\n");

				//case 4
				fprintf(fp, "\\dd_%d_%d_4: \n", p->second.get_ID(), cur_child->get_ID());
				bool whether1stline7 = true;
				for (int i = cur_child->get_ASAP(); i <= cur_child->get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							if (whether1stline7)
							{
								fprintf(fp, " %d Z(%d,%d,%d,%d)", i, cur_child->get_ID(), i, j, k);
								whether1stline7 = false;
							}
							else
								fprintf(fp, " + %d Z(%d,%d,%d,%d)", i, cur_child->get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				for (int i = p->second.get_ASAP(); i <= p->second.get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " - %d Z(%d,%d,%d,%d)", i, p->second.get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}

				//(4),(a-b)+(c-d) = a -b + c - d
				for (int i = cur_child->get_ASAP(); i <= cur_child->get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " + %d Z(%d,%d,%d,%d)", j, cur_child->get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				for (int i = p->second.get_ASAP(); i <= p->second.get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " - %d Z(%d,%d,%d,%d)", j, p->second.get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				for (int i = cur_child->get_ASAP(); i <= cur_child->get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " + %d Z(%d,%d,%d,%d)", k, cur_child->get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				for (int i = p->second.get_ASAP(); i <= p->second.get_wd_ALAP(); i++) {
					for (int j = 0; j <= M_loc_x; j++) {
						for (int k = 0; k <= M_loc_y; k++) {
							fprintf(fp, " - %d Z(%d,%d,%d,%d)", k, p->second.get_ID(), i, j, k);
						}
					}
					fprintf(fp, "\n");
				}
				fprintf(fp, ">=1\n");
			}
		}

		//resource constraint
		fprintf(fp, "\\Resource constraint\n");
		for (int x = 0; x <= M_loc_x; x++) {
			for (int y = 0; y <= M_loc_y; y++) {
				fprintf(fp, "\\FU_%d%d:\n", x, y);
				for (int cs = 0; cs<II; cs++) {
					fprintf(fp, "\\CS_%d: \n", cs);
					bool whether1stline9 = true;
					for (p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
						DFG_vertex *cur_vertex;
						cur_vertex = &(*p).second;
						for (int sch = cur_vertex->get_ASAP(); sch <= cur_vertex->get_wd_ALAP(); sch++) {
							if (sch%II == cs)
								if (whether1stline9)
								{
									fprintf(fp, " Z(%d,%d,%d,%d)", cur_vertex->get_ID(), sch, x, y);
									whether1stline9 = false;
								}
								else
									fprintf(fp, " + Z(%d,%d,%d,%d)", cur_vertex->get_ID(), sch, x, y);
						}
						fprintf(fp, "\n");
					}
					fprintf(fp, "<=1\n");
				}
			}
		}

		//Bounds
		fprintf(fp, "\n Bounds\n\n");
		for (p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
			DFG_vertex *cur_vertex;
			cur_vertex = &(*p).second;
			for (int sch = cur_vertex->get_ASAP(); sch <= cur_vertex->get_wd_ALAP(); sch++) {
				for (int x = 0; x <= M_loc_x; x++) {
					for (int y = 0; y <= M_loc_y; y++) {
						fprintf(fp, "0 <= Z(%d,%d,%d,%d) <= 1\n", cur_vertex->get_ID(), sch, x, y);
					}
				}
			}
		}

		//Binary
		fprintf(fp, "\n BINARY\n");
		int space = 0;
		for (p = DFG->Vertices.begin(); p != DFG->Vertices.end(); p++) {
			DFG_vertex *cur_vertex;
			cur_vertex = &(*p).second;
			for (int sch = cur_vertex->get_ASAP(); sch <= cur_vertex->get_wd_ALAP(); sch++) {
				for (int x = 0; x <= M_loc_x; x++) {
					for (int y = 0; y <= M_loc_y; y++) {
						space++;
						fprintf(fp, "Z(%d,%d,%d,%d)   ", cur_vertex->get_ID(), sch, x, y);
						if (space % 4 == 1) { fprintf(fp, "\n"); }
					}
				}
			}
		}
		fprintf(fp, "\n end\n");
		fclose(fp);
	}




	void SwingModuloScheduler::SMSchedule(char *filename) {
		//	std::vector<DFG_vertex*>::iterator r;
		char temp[50];
		printf("Analysis Start...\n");
		this->Analyze_DFG();
		printf("Analysis End...\n\n");
		sprintf(temp, "%s.png", filename);
		DFG->OutputPNG(temp);
		printf("Output LP_file begins:\n");
		this->PrintLPfile();
		printf("Output LP_file ends:\n");
	}

	void SwingModuloScheduler::OutputHTML(char *filename, int II) {
		HardWare->OutputHTML(filename, II);
	}
};








