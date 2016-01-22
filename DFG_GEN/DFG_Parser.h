#pragma once
// parsertest5.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//
//[Parsertest2.cpp]
//Summary
//	read stdin input stream
//	parse stream
//	recognise 1output 3input instruction
//	recognise variable label (started with '%')
//	Detect flow-dependency
//	Make CFG-virtex for each instruction-word
//	Make CFG-edge for each flow-dependency
//
//Bugfix History
//	fixed bug in the status constants of parser operation
//	#define PARSE_OP2_DONE		 3		
//=>#define PARSE_OP2_DONE		 4

//[Parsertest3.cpp]
//Summary (20050520_1058)
//	Parsertest2 operation
//	detect memory-variable(pointer, array) dependency
//		detect memory access of getelementptr, select, load, store instruction
//	detect RAW, WAR, WAW hazard
//	Make CFG-edge for each memory hazard
//	

//[Parsertest4.cpp]
//Summary
//	Parsertest3 operation
//	list up unsolved input variables
//	detect loop-iteration dependecy from local variable
//	recognise 1output 4input instruction

//[Parsertest5.cpp]
//Summary
//	Parsertest4 operation
//	modified string-parser operation
//		recognize pointer-type
//		added FLAG_INPUTx_PTR return-flag
//	recognise non-pointer handling 'select' instruction 


#include "operators.h"
#include "DFG.h"
#include "Mdependencylist.h"
#include <vector>
//parser return value
#define FLAG_PARSE_SUCCESS	1
#define FLAG_OUTPUT_ENABLE	2
#define FLAG_INPUT1_ENABLE	4
#define FLAG_INPUT2_ENABLE	8

#define FLAG_INPUT3_ENABLE	16
#define FLAG_INPUT4_ENABLE	32
//reserved flag				64
#define FLAG_INPUT1_PTR		128

#define FLAG_INPUT2_PTR		256
#define FLAG_INPUT3_PTR		512
#define FLAG_INPUT4_PTR		1024
//reserved flag				2048
//parser internal values
#define PARSING_NONE         0
#define PARSING_VALLABEL     1
#define PARSING_INSTRUCTION  2

#define PARSE_INST_DONE      1
#define PARSE_OP1_DONE		 2
#define PARSE_OP2_DONE		 4
#define PARSE_OP3_DONE		 8
#define PARSE_OP4_DONE		 16

class outlabeldata {
	//class to store variable label of output value
	//each output value label is associated with virtex id-number,
	int id;
	char valname[32];
public:
	int outlabeldata::getid(void) { return id; }
	char *outlabeldata::getname(void) { return valname; }
	outlabeldata::outlabeldata(int idnum, char* label)
	{
		strncpy(valname, label, 31);
		valname[31] = '\0';
		id = idnum;
	}
};

class unsolvedinputdata {
	char valname[32];
	int line_firstread;
	int line_lastwrite;
	int resolved;
public:
	unsolvedinputdata::unsolvedinputdata(int lineno, char *label) {
		strncpy(valname, label, 31);
		valname[31] = '\0';
		line_firstread = lineno;
		line_lastwrite = -1;
		resolved = 0;
	}
	char *unsolvedinputdata::getname() { return valname; }
	int unsolvedinputdata::isresolved() { return resolved; }
	int unsolvedinputdata::getwritelineno() { return line_lastwrite; }
	int unsolvedinputdata::getreadlineno() { return line_firstread; }
	void unsolvedinputdata::putwriteinfo(int lineno) {
		line_lastwrite = lineno;
		resolved = 1;
	}
};


class DFG_maker {
private:
	DataFlowGraph DFG0;

	void DFG_maker::print_unsolvedinputlist(std::vector <unsolvedinputdata> &usilist) {
		std::vector <unsolvedinputdata>::iterator pusidat;
		printf("print unsolved input variable list\n");
		for (pusidat = usilist.begin(); pusidat != usilist.end(); pusidat++) {
			printf("%s: %d: %d: %d\n", pusidat->getname(), pusidat->getreadlineno(),
				pusidat->getwritelineno(), pusidat->isresolved());
		}
	}

	void DFG_maker::print_parseresult(int resultflag, int inst_no, char *inst_str, char *op0_str, char *op1_str, char *op2_str, char *op3_str, char *op4_str) {
		printf("result_0x%X ", resultflag);
		printf("f%d_%s ", inst_no, inst_str);
		if (resultflag & FLAG_OUTPUT_ENABLE) {
			printf("o_%s ", op0_str);
		}
		if (resultflag & FLAG_INPUT1_ENABLE) {
			printf("i1_%s ", op1_str);
		}
		if (resultflag & FLAG_INPUT2_ENABLE) {
			printf("i2_%s ", op2_str);
		}
		if (resultflag & FLAG_INPUT3_ENABLE) {
			printf("i3_%s ", op3_str);
		}
		if (resultflag & FLAG_INPUT4_ENABLE) {
			printf("i4_%s ", op4_str);
		}
		putchar('\n');
	}


	int  DFG_maker::handle_inputvar(DataFlowGraph &DFG, std::vector <outlabeldata> &olist, std::vector<unsolvedinputdata> &ilist, char *labelname, int lineno)
	{
		int sublinecounter = 0;
		int inputvar_solved = 0;
		std::vector <outlabeldata>::iterator poldat;
		for (poldat = olist.begin(); poldat != olist.end(); poldat++)
		{
			if (!strcmp(poldat->getname(), labelname)) {
				printf("3:%d-%d\n", sublinecounter, lineno);
				DFG.draw_edge(sublinecounter, lineno);
				inputvar_solved = 1;
			}
			sublinecounter++;
		}
		if (!inputvar_solved) {
			//input variable unsolved
			unsolvedinputdata temp(lineno, labelname);
			ilist.push_back(temp);
		}
		return 1;
	}




	int  DFG_maker::parseline(const char *instline, int *inst_number, char *inst_str, char *op0_str, char *op1_str, char *op2_str, char *op3_str, char *op4_str) {
		//parsable expression format
		//%(outvar) (command) %(invar) %(invar)
		//delimitter letter is \t (space)
		int parsepos = 0;
		int parseresult = 0;
		int parserstatus = PARSING_NONE;
		int parseroperation = 0;
		int midbracedepth = 0;
		int mode_readmidbrace = 0;

		char tempbuffer[32];
		int tempbufferpos = 0;
		char tempchar;
		*inst_number = 0;

		while (1) {
			tempchar = instline[parsepos++];
			tempchar = toupper(tempchar);
			switch (tempchar) {
			case '\t':
			case ' ':
			case '=':
			case ';':
			case ',':
			case '\n':
			case '\r':
			case '\0':
				if (midbracedepth && (!mode_readmidbrace)) {
					if (tempchar == ';') {
						return parseresult;
					}
					if (tempchar == '\0') {
						return parseresult;
					}
					break;
				}
				tempbuffer[tempbufferpos] = '\0';
				if (parserstatus == PARSING_VALLABEL) {
					if (parseroperation & PARSE_INST_DONE) {
						//found valuable before instruction
						if (parseroperation & PARSE_OP3_DONE) {
							strcpy(op4_str, tempbuffer);
							parseresult |= FLAG_INPUT4_ENABLE;
						}
						else if (parseroperation & PARSE_OP2_DONE) {
							strcpy(op3_str, tempbuffer);
							parseresult |= FLAG_INPUT3_ENABLE;
						}
						else if (parseroperation & PARSE_OP1_DONE) {
							strcpy(op2_str, tempbuffer);
							parseresult |= FLAG_INPUT2_ENABLE;
						}
						else {
							strcpy(op1_str, tempbuffer);
							parseresult |= FLAG_INPUT1_ENABLE;
						}
					}
					else {
						//found valuable before instruction
						strcpy(op0_str, tempbuffer);
						parseresult |= FLAG_OUTPUT_ENABLE;
					}
				}
				else if (parserstatus == PARSING_INSTRUCTION) {
					if (parseroperation & PARSE_INST_DONE) {
						//identifier or literal-value
					}
					else {
						strcpy(inst_str, tempbuffer);
						*inst_number = getinstnum(inst_str);
						if (*inst_number == OP_NUM_PHI) {
							mode_readmidbrace = 1;
						}
						parseroperation |= PARSE_INST_DONE;
						parseresult |= FLAG_PARSE_SUCCESS;
					}
				}
				if (tempchar == ',') {
					if (parseroperation & PARSE_OP3_DONE) {
						parseroperation |= PARSE_OP4_DONE;
					}
					if (parseroperation & PARSE_OP2_DONE) {
						parseroperation |= PARSE_OP3_DONE;
					}
					if (parseroperation & PARSE_OP1_DONE) {
						parseroperation |= PARSE_OP2_DONE;
					}
					if (parseroperation & PARSE_INST_DONE) {
						parseroperation |= PARSE_OP1_DONE;
					}
				}
				tempbufferpos = 0;
				parserstatus = PARSING_NONE;
				if (tempchar == ';') {
					return parseresult;
				}
				if (tempchar == '\0') {
					return parseresult;
				}
				break;
			case '[':
				midbracedepth++;
				break;
			case ']':
				if (midbracedepth) {
					midbracedepth--;
				}
				else {
					//midbracedepth = 0
					printf("parseline:error:orphan ]\n");
				}
				break;
			case '%':
				if (midbracedepth && (!mode_readmidbrace)) {
					break;
				}
				if (tempbufferpos != 0) {
					//error: found '%' in the token 
					return parseresult;
				}
				parserstatus = PARSING_VALLABEL;
				tempbuffer[tempbufferpos++] = tempchar;
				break;
			case '*':
				if (midbracedepth && (!mode_readmidbrace)) {
					break;
				}
				if (parserstatus != PARSING_VALLABEL) {
					parserstatus = PARSING_INSTRUCTION;
				}
				if (parseroperation & PARSE_OP3_DONE) {
					parseresult |= FLAG_INPUT4_PTR;
				}
				else if (parseroperation & PARSE_OP2_DONE) {
					parseresult |= FLAG_INPUT3_PTR;
				}
				else if (parseroperation & PARSE_OP1_DONE) {
					parseresult |= FLAG_INPUT2_PTR;
				}
				if (parseroperation & PARSE_INST_DONE) {
					parseresult |= FLAG_INPUT1_PTR;
				}

				tempbuffer[tempbufferpos++] = tempchar;
				break;
			default:
				//got letter
				if (midbracedepth && (!mode_readmidbrace)) {
					break;
				}
				if (parserstatus != PARSING_VALLABEL) {
					parserstatus = PARSING_INSTRUCTION;
				}
				tempbuffer[tempbufferpos++] = tempchar;
				break;
			}
		}
		return 0;
	}


	int  DFG_maker::getinsttype(int inst_num) {
		switch (inst_num) {
		case OP_NUM_STORE:
			return STORE;

		case OP_NUM_LOAD:
			return LOAD;

		case OP_NUM_ADD:
		case OP_NUM_SUB:
		case OP_NUM_AND:
		case OP_NUM_OR:
		case OP_NUM_XOR:
		case OP_NUM_GETELEMENTPTR:
			return ADD;

		case OP_NUM_MUL:
			return MULT;

		case OP_NUM_DIV:
		case OP_NUM_REM:
			return DIV;

		case OP_NUM_SHL:
		case OP_NUM_SHR:
			return SHIFT;

		case OP_NUM_SETEQ:
		case OP_NUM_SETNE:
		case OP_NUM_SETLE:
		case OP_NUM_SETGE:
		case OP_NUM_SETLT:
		case OP_NUM_SETGT:
		case OP_NUM_MALLOC:
		case OP_NUM_FREE:
		case OP_NUM_ALLOCA:
			return MOV;

		case OP_NUM_RET:
		case OP_NUM_CALL:
			return JUMP;

		case OP_NUM_BR:
		case OP_NUM_SWITCH:
		case OP_NUM_INVOKE:
			return CJUMP;

		case OP_NUM_UNWIND:
		case OP_NUM_UNREACHABLE:
		case OP_NUM_PHI:
		case OP_NUM_CAST:
		case OP_NUM_VANEXT:
		case OP_NUM_VAARG:
		case OP_NUM_SELECT:
			return NOP;

		default:
			return NOP;
		}
	}

	int  DFG_maker::getinstnum(const char *inst_str) {
		if (!strcmp("RET", inst_str)) { return OP_NUM_RET; }
		if (!strcmp("BR", inst_str)) { return OP_NUM_BR; }
		if (!strcmp("SWITCH", inst_str)) { return OP_NUM_SWITCH; }
		if (!strcmp("INVOKE", inst_str)) { return OP_NUM_INVOKE; }
		if (!strcmp("UNWIND", inst_str)) { return OP_NUM_UNWIND; }
		if (!strcmp("UNREACHABLE", inst_str)) { return OP_NUM_UNREACHABLE; }
		if (!strcmp("ADD", inst_str)) { return OP_NUM_ADD; }
		if (!strcmp("SUB", inst_str)) { return OP_NUM_SUB; }
		if (!strcmp("MUL", inst_str)) { return OP_NUM_MUL; }
		if (!strcmp("DIV", inst_str)) { return OP_NUM_DIV; }
		if (!strcmp("REM", inst_str)) { return OP_NUM_REM; }
		if (!strcmp("AND", inst_str)) { return OP_NUM_AND; }
		if (!strcmp("OR", inst_str)) { return OP_NUM_OR; }
		if (!strcmp("XOR", inst_str)) { return OP_NUM_XOR; }
		if (!strcmp("SETEQ", inst_str)) { return OP_NUM_SETEQ; }
		if (!strcmp("SETNE", inst_str)) { return OP_NUM_SETNE; }
		if (!strcmp("SETLE", inst_str)) { return OP_NUM_SETLE; }
		if (!strcmp("SETGE", inst_str)) { return OP_NUM_SETGE; }
		if (!strcmp("SETLT", inst_str)) { return OP_NUM_SETLT; }
		if (!strcmp("SETGT", inst_str)) { return OP_NUM_SETGT; }
		if (!strcmp("MALLOC", inst_str)) { return OP_NUM_MALLOC; }
		if (!strcmp("FREE", inst_str)) { return OP_NUM_FREE; }
		if (!strcmp("ALLOCA", inst_str)) { return OP_NUM_ALLOCA; }
		if (!strcmp("LOAD", inst_str)) { return OP_NUM_LOAD; }
		if (!strcmp("STORE", inst_str)) { return OP_NUM_STORE; }
		if (!strcmp("GETELEMENTPTR", inst_str)) { return OP_NUM_GETELEMENTPTR; }
		if (!strcmp("PHI", inst_str)) { return OP_NUM_PHI; }
		if (!strcmp("CAST", inst_str)) { return OP_NUM_CAST; }
		if (!strcmp("CALL", inst_str)) { return OP_NUM_CALL; }
		if (!strcmp("SHL", inst_str)) { return OP_NUM_SHL; }
		if (!strcmp("SHR", inst_str)) { return OP_NUM_SHR; }
		if (!strcmp("VANEXT", inst_str)) { return OP_NUM_VANEXT; }
		if (!strcmp("VAARG", inst_str)) { return OP_NUM_VAARG; }
		if (!strcmp("SELECT", inst_str)) { return OP_NUM_SELECT; }
		return 0;
	}
public:
	DFG_maker::DFG_maker() {};
	int  DFG_maker::analyze_ll(const char *filename) {
		std::vector <outlabeldata> oldatlist;
		std::vector <outlabeldata>::iterator poldat;
		std::vector <unsolvedinputdata> usilist;
		std::vector <unsolvedinputdata>::iterator pusidat;
		memvariablelist mvlst;
		std::vector <int> loadaccesslinelist;
		std::vector <int> storeaccesslinelist;

		char linebuffer[256];
		int instruction_number;
		int linecounter = 0;
		int sublinecounter = 0;
		int inputvar_solved = 0;
		char instruction_str[32];
		char operand0[32];
		char operand1[32];
		char operand2[32];
		char operand3[32];
		char operand4[32];
		int parse_res;
		FILE *fp;

		if ((fp = fopen(filename, "r")) == NULL) {
			printf("file open error!! [%s]\n", filename);
			exit(1);
		}
		printf("%s has opend.\n", filename);


		oldatlist.clear();
		while (!feof(fp)) {
			//gets(linebuffer);
			fgets(linebuffer, 256, fp);
			parse_res = parseline(linebuffer, &instruction_number, instruction_str,
				operand0, operand1, operand2, operand3, operand4);
			if (parse_res & FLAG_PARSE_SUCCESS) {
				DFG0.add_vertex(linecounter, instruction_str, getinsttype(instruction_number));
				if (!(parse_res & FLAG_OUTPUT_ENABLE)) {
					operand0[0] = '\0';
				}
				print_parseresult(parse_res, instruction_number, instruction_str,
					operand0, operand1, operand2, operand3, operand4);
				//handle input variables
				if (parse_res & FLAG_INPUT1_ENABLE) {
					handle_inputvar(DFG0, oldatlist, usilist, operand1, linecounter);
				}
				if (parse_res & FLAG_INPUT2_ENABLE) {
					handle_inputvar(DFG0, oldatlist, usilist, operand2, linecounter);
				}
				if (parse_res & FLAG_INPUT3_ENABLE) {
					handle_inputvar(DFG0, oldatlist, usilist, operand3, linecounter);
				}
				if (parse_res & FLAG_INPUT4_ENABLE) {
					handle_inputvar(DFG0, oldatlist, usilist, operand4, linecounter);
				}
				//add oldatlist
				outlabeldata temp(linecounter, operand0);
				oldatlist.push_back(temp);
				//detect local variable dependency
				for (pusidat = usilist.begin(); pusidat != usilist.end(); pusidat++)
				{
					if (!strcmp(pusidat->getname(), operand0)) {
						//found
						if (pusidat->isresolved()) {
							//draw output dependency edge
							DFG0.draw_edge(pusidat->getwritelineno(), linecounter);
						}
						//draw anti dependency edge
						DFG0.draw_edge(pusidat->getreadlineno(), linecounter);
						pusidat->putwriteinfo(linecounter);
					}
				}
				//detect memory dependency, detect loop dependency
				switch (instruction_number) {
				case OP_NUM_SELECT:
					if (!(parse_res & FLAG_OUTPUT_ENABLE)) {
						//could not find output variable label
						printf("line%d:fatal parse error:output label not found\n", linecounter);
						break;
					}
					if ((parse_res & FLAG_INPUT2_ENABLE) && (parse_res & FLAG_INPUT2_PTR)) {
						//found alias?? variable labelname
						mvlst.addselectalias(operand0, operand2);
					}
					if ((parse_res & FLAG_INPUT3_ENABLE) && (parse_res & FLAG_INPUT3_PTR)) {
						//found alias?? variable labelname
						mvlst.addselectalias(operand0, operand3);
					}
					break;
				case OP_NUM_GETELEMENTPTR:
					if (!(parse_res & FLAG_OUTPUT_ENABLE)) {
						//could not find output variable label
						printf("line%d:fatal parse error:output label not found\n", linecounter);
						break;
					}
					if (parse_res & FLAG_INPUT1_ENABLE) {
						mvlst.addaliasname(operand1, operand0);
					}
					break;
				case OP_NUM_LOAD:
					if (parse_res & FLAG_INPUT1_ENABLE) {
						std::vector <int>::iterator lin1;
						mvlst.loadaccessprocess(operand1, storeaccesslinelist,
							loadaccesslinelist, linecounter);
						//detect all the preceeding store instruction
						//(detect RAW hazard)
						for (lin1 = storeaccesslinelist.begin(); lin1 != storeaccesslinelist.end(); lin1++)
						{
							if (*lin1 != linecounter) {
								//found RAW hazard
								DFG0.draw_edge(linecounter, *lin1, 1);
								DFG0.draw_edge(*lin1, linecounter);
							}
						}
						if (!storeaccesslinelist.empty()) {
							for (lin1 = loadaccesslinelist.begin(); lin1 != loadaccesslinelist.end(); lin1++)
							{
								if (*lin1 != linecounter) {
									//found RAW hazard
									DFG0.draw_edge(linecounter, *lin1, 1);
									DFG0.draw_edge(*lin1, linecounter);
								}
							}
						}
					}
					break;
				case OP_NUM_STORE:
					if (parse_res & FLAG_INPUT2_ENABLE) {
						std::vector <int>::iterator lin1;
						mvlst.storeaccessprocess(operand2, storeaccesslinelist,
							loadaccesslinelist, linecounter);
						//detect all the preceeding load store instruction
						//(detect WAW hazard)
						for (lin1 = storeaccesslinelist.begin(); lin1 != storeaccesslinelist.end(); lin1++)
						{
							if (*lin1 != linecounter) {
								//found WAW hazard
								DFG0.draw_edge(linecounter, *lin1, 1);
								DFG0.draw_edge(*lin1, linecounter);
							}
						}
						//(detect WAR hazard)
						for (lin1 = loadaccesslinelist.begin(); lin1 != loadaccesslinelist.end(); lin1++)
						{
							if (*lin1 != linecounter) {
								//found WAR hazard
								DFG0.draw_edge(linecounter, *lin1, 1);
								DFG0.draw_edge(*lin1, linecounter);
							}
						}
					}
					break;
				}

				linecounter++;
			}
			else {
				//parser operation failed
				printf("parse_fail:%s\n", linebuffer);
			}
		}
		//end of parse operation
		//culcurate local variable inter-loop dependency
		for (pusidat = usilist.begin(); pusidat != usilist.end(); pusidat++) {
			if (pusidat->isresolved()) {
				//inter-loop dependency
				DFG0.draw_edge(pusidat->getwritelineno(), pusidat->getreadlineno(), 1);
			}
		}

		//	DFG0.print_DFG();
		mvlst.printmemvarlist();
		//	print_unsolvedinputlist(usilist);
		//	DFG0.OutputPNG("parse2sample.png");
		//	DFG0.OutputStatement("parse2sample.c");
		fclose(fp);
		return 0;
	}

	DataFlowGraph DFG_maker::get_DFG() { return DFG0; }
};

