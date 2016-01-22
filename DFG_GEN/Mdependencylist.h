//This file describe a class to handle
//inter-loop dependency informations of memory access

#include <vector>

using namespace std;

//Class information
//
//Hierarchy of class variavbles
//
//class [memvariablelist]
//----class [memvariableinfo]
//--------string [real name of array or pointer variable]
//--------string list [alias names of real variable]
//--------int list [linenumbers of load access instruction]
//--------int list [linenumbers of store access instruction]



class memvariableinfo
{
private:
	char realvarname[32];
	std::vector <char *> aliasvarnames;
	std::vector <int> loadaccesslist;
	std::vector <int> storeaccesslist;
public:
	memvariableinfo(char *newrealname)
	{
		aliasvarnames.clear();
		loadaccesslist.clear();
		storeaccesslist.clear();
		strncpy(realvarname, newrealname, 31);
		realvarname[31] = '\0';
	}

	int addaliasname(char *newaliasname) {
		int namesize = (int)strlen(newaliasname);
		if (namesize == 0)
		{
			return 0;
		}
		char * newnamebuff = (char *)malloc(sizeof(char) * namesize + 1);
		if (newnamebuff == 0) {
			return 0;
		}
		strcpy(newnamebuff, newaliasname);
		aliasvarnames.push_back(newnamebuff);		//must be freed when the class-object get deleted
		return 1;
	}

	char *getrealvarname(void)
	{
		return realvarname;
	}

	int existaliasname(char* aname)
	{
		std::vector <char *>::iterator pstr1;
		for (pstr1 = aliasvarnames.begin(); pstr1 != aliasvarnames.end(); pstr1++)
		{
			if (!strcmp(aname, *pstr1)) {
				return 1;
			}
		}
		return 0;
	}

	char *alias2real(char *aname)
	{
		if (existaliasname(aname)) {
			return getrealvarname();
		}
		return 0;
	}

	int addstoreaccess(int lnum)
	{
		//if the same line-number already exists -> return 0
		storeaccesslist.push_back(lnum);
		return 1;
	}

	int addloadaccess(int lnum)
	{
		//if the same line-number already exists -> return 0
		loadaccesslist.push_back(lnum);
		return 1;
	}

	void printmemvarinfo(void)
	{
		std::vector <char *>::iterator pstr1;
		std::vector <int>::iterator pnum1;
		int itemcount = 0;
		printf("array realname: %s\n", realvarname);
		printf("array aliasnames:\n");
		for (pstr1 = aliasvarnames.begin(); pstr1 != aliasvarnames.end(); pstr1++)
		{
			printf("[%d] %s\n", itemcount, *pstr1);
			itemcount++;
		}
		if (itemcount == 0) {
			printf("    none\n");
		}
		itemcount = 0;
		printf("load access instructions\n");
		for (pnum1 = loadaccesslist.begin(); pnum1 != loadaccesslist.end(); pnum1++) {
			printf("  %d", *pnum1);
			itemcount++;
		}
		if (itemcount == 0) {
			printf("    none\n");
		}
		else {
			putchar('\n');
		}
		itemcount = 0;
		printf("store access instructions\n");
		for (pnum1 = storeaccesslist.begin(); pnum1 != storeaccesslist.end(); pnum1++) {
			printf("  %d", *pnum1);
			itemcount++;
		}
		if (itemcount == 0) {
			printf("    none\n");
		}
		else {
			putchar('\n');
		}
	}

	std::vector <int> *getstorelinelist() {
		return &storeaccesslist;
	}

	std::vector <int> *getloadlinelist() {
		return &loadaccesslist;
	}

};		//end of class memvariableinfo

class memvariablelist
{
private:
	std::vector <memvariableinfo> memvarpages;
public:
	int addentry(char *realarrayname)
	{
		memvariableinfo *newpage = new memvariableinfo(realarrayname);
		memvarpages.push_back(*newpage);
		return 1;
	}

	int addaliasname(char *rname, char *naname)
	{
		std::vector <memvariableinfo> ::iterator mvi1;
		char *realnamebuff;
		printf("addaliasname called: %s, %s\n", rname, naname);
		for (mvi1 = memvarpages.begin(); mvi1 != memvarpages.end(); mvi1++)
		{
			//realname exist?
			realnamebuff = mvi1->getrealvarname();
			if (!strcmp(rname, realnamebuff)) {
				if (mvi1->existaliasname(naname)) {
					return 2;
				}
				else {
					mvi1->addaliasname(naname);
					return 1;
				}
			}
		}
		//realname does not exist
		memvariableinfo *newpage = new memvariableinfo(rname);
		newpage->addaliasname(naname);
		memvarpages.push_back(*newpage);
		return 0;
	}

	char *alias2real(char *aname)
	{
		std::vector <memvariableinfo> ::iterator mvi1;
		char *realnamebuff;
		for (mvi1 = memvarpages.begin(); mvi1 != memvarpages.end(); mvi1++)
		{
			realnamebuff = mvi1->alias2real(aname);
			if (realnamebuff != 0) {
				return realnamebuff;
			}
		}
		return 0;
	}

	int existrealname(char *rname)
	{
		std::vector <memvariableinfo> ::iterator mvi1;
		char *realnamebuff;
		for (mvi1 = memvarpages.begin(); mvi1 != memvarpages.end(); mvi1++)
		{
			realnamebuff = mvi1->getrealvarname();
			if (!strcmp(rname, realnamebuff)) {
				return 1;
			}
		}
		return 0;
	}

	int addselectalias(char *selectresult, char *selectsource)
	{
		int flg_aliasfound = 0;
		//check if selectsource is realname of the array
		if (existrealname(selectsource)) {
			addaliasname(selectsource, selectresult);
			return 1;
		}
		else {
			//switchsource was aliasname
			std::vector <memvariableinfo>::iterator mvi1;
			for (mvi1 = memvarpages.begin(); mvi1 != memvarpages.end(); mvi1++)
			{
				if (mvi1->existaliasname(selectsource)) {
					mvi1->addaliasname(selectresult);
					flg_aliasfound = 1;
				}
			}
			if (flg_aliasfound) {
				return 2;
			}
			else {
				addaliasname(selectsource, selectresult);
				return 0;
			}
		}
	}
	int storeaccessprocess(char *addrname, std::vector <int> &storelist, std::vector <int> &loadlist, int lineno)
	{
		int flg_aliasaddr = 0;
		char *realnamebuf;
		std::vector <int> *pstlst;
		std::vector <int> *plolst;
		std::vector <int>::iterator copy_ntemp;
		storelist.clear();
		loadlist.clear();
		std::vector <memvariableinfo> ::iterator mvi1;
		for (mvi1 = memvarpages.begin(); mvi1 != memvarpages.end(); mvi1++)
		{
			realnamebuf = mvi1->getrealvarname();
			if (!strcmp(realnamebuf, addrname)) {
				//found addrname as realname
				pstlst = mvi1->getstorelinelist();
				plolst = mvi1->getloadlinelist();
				for (copy_ntemp = (*pstlst).begin(); copy_ntemp != (*pstlst).end(); copy_ntemp++)
				{
					storelist.push_back(*copy_ntemp);
				}
				for (copy_ntemp = (*plolst).begin(); copy_ntemp != (*plolst).end(); copy_ntemp++)
				{
					loadlist.push_back(*copy_ntemp);
				}
				mvi1->addstoreaccess(lineno);
				return 2;
			}
			if (mvi1->existaliasname(addrname)) {
				//found addrname as aliasname
				pstlst = mvi1->getstorelinelist();
				plolst = mvi1->getloadlinelist();
				for (copy_ntemp = (*pstlst).begin(); copy_ntemp != (*pstlst).end(); copy_ntemp++)
				{
					storelist.push_back(*copy_ntemp);
				}
				for (copy_ntemp = (*plolst).begin(); copy_ntemp != (*plolst).end(); copy_ntemp++)
				{
					loadlist.push_back(*copy_ntemp);
				}
				mvi1->addstoreaccess(lineno);
				flg_aliasaddr = 1;
			}
		}
		if (flg_aliasaddr) {
			return 1;
		}
		//completely not found => new realname
		memvariableinfo *newpage = new memvariableinfo(addrname);
		(*newpage).addstoreaccess(lineno);
		memvarpages.push_back(*newpage);
		return 4;
	}
	int loadaccessprocess(char *addrname, std::vector <int> &storelist, std::vector <int> &loadlist, int lineno)
	{
		int flg_aliasaddr = 0;
		char *realnamebuf;
		std::vector <int> *pstlst;
		std::vector <int> *plolst;
		std::vector <int>::iterator copy_ntemp;
		storelist.clear();
		loadlist.clear();
		std::vector <memvariableinfo> ::iterator mvi1;
		for (mvi1 = memvarpages.begin(); mvi1 != memvarpages.end(); mvi1++)
		{
			realnamebuf = mvi1->getrealvarname();
			if (!strcmp(realnamebuf, addrname)) {
				//found addrname as realname
				pstlst = mvi1->getstorelinelist();
				plolst = mvi1->getloadlinelist();
				for (copy_ntemp = (*pstlst).begin(); copy_ntemp != (*pstlst).end(); copy_ntemp++)
				{
					storelist.push_back(*copy_ntemp);
				}
				for (copy_ntemp = (*plolst).begin(); copy_ntemp != (*plolst).end(); copy_ntemp++)
				{
					loadlist.push_back(*copy_ntemp);
				}
				mvi1->addloadaccess(lineno);
				return 2;
			}
			if (mvi1->existaliasname(addrname)) {
				//found addrname as aliasname
				pstlst = mvi1->getstorelinelist();
				plolst = mvi1->getloadlinelist();
				for (copy_ntemp = (*pstlst).begin(); copy_ntemp != (*pstlst).end(); copy_ntemp++)
				{
					storelist.push_back(*copy_ntemp);
				}
				for (copy_ntemp = (*plolst).begin(); copy_ntemp != (*plolst).end(); copy_ntemp++)
				{
					loadlist.push_back(*copy_ntemp);
				}
				mvi1->addloadaccess(lineno);
				flg_aliasaddr = 1;
			}
		}
		if (flg_aliasaddr) {
			return 1;
		}
		//completely not found => new realname
		memvariableinfo *newpage = new memvariableinfo(addrname);
		(*newpage).addloadaccess(lineno);
		memvarpages.push_back(*newpage);
		return 4;
	}

	int addstoreline(char *rname, int lnum)
	{
		std::vector <memvariableinfo> ::iterator mvi1;
		char *realnamebuff;
		for (mvi1 = memvarpages.begin(); mvi1 != memvarpages.end(); mvi1++)
		{
			realnamebuff = mvi1->getrealvarname();
			if (!strcmp(rname, realnamebuff)) {
				mvi1->addstoreaccess(lnum);
				return 1;
			}
		}
		return 0;
	}

	int addloadline(char *rname, int lnum)
	{
		std::vector <memvariableinfo> ::iterator mvi1;
		char *realnamebuff;
		for (mvi1 = memvarpages.begin(); mvi1 != memvarpages.end(); mvi1++)
		{
			realnamebuff = mvi1->getrealvarname();
			if (!strcmp(rname, realnamebuff)) {
				mvi1->addloadaccess(lnum);
				return 1;
			}
		}
		return 0;
	}

	void printmemvarlist(void)
	{
		std::vector <memvariableinfo> ::iterator mvi1;
		int elemcount = 0;
		printf("----start of printmemverlist----\n");
		for (mvi1 = memvarpages.begin(); mvi1 != memvarpages.end(); mvi1++)
		{
			printf("----element %d----\n", elemcount);
			mvi1->printmemvarinfo();
			elemcount++;
		}
		printf("----end of printmemvarlist----\n");
	}
};
