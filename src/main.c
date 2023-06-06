/*
shahar bar natan - 315638742 || tal hadaya - 318637543
*/

#include "main.h"

/*global vatibales*/
int ic, dc, err;
unsigned int data[MACHINE_RAM];
unsigned int instructions[MACHINE_RAM];
const char base32[32] = {'!','@','#','$','%','^','&','*','<','>','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v'};
const char *commands[] = {"mov", "cmp", "add", "sub", "not", "clr", "lea", "inc", "dec", "jmp", "bne","get", "prn", "jsr", "rts", "hlt"};
const char *directives[] = {".data", ".string", ".struct", ".entry", ".extern"};
labelPtr symbolsTable;
extPtr extList;
boolean entryExists, externExists, wasError;

int main(int argc, char const *argv[])
{
	int i, flag;
	char *fullFileName = NULL, *mactoName = NULL;	
	FILE * file;
	/*initialize macro list*/
	struct  Macro* Mtail= NULL;
	struct  Macro* Mhead = NULL;
	Mhead = (struct Macro*)malloc(sizeof(struct Macro));
	Mtail = (struct Macro*)malloc(sizeof(struct Macro));
	Mtail = Mhead;
	
	/*check if user put the files if not exit*/
	if(argc <= 1)
	{
		printf("No File where typed.\n");
		exit(0);
	}
	
	for(i=1; i<argc; i++)
	{	
		fullFileName = NULL;
		mactoName = NULL;
		fullFileName = asFile(argv[i]);
		mactoName = amFile(argv[i]);
		printf("************* Started %s assembling process *************\n\n", fullFileName);
		file = fopen(fullFileName, "r");
		flag = PreReadFile(fullFileName,Mhead);
		if (!flag)
		{
			PreWriteFile(fullFileName,mactoName,Mtail);	
		}
		else
			printf("file %s does not exist", fullFileName);
		file = fopen(mactoName, "r");
		resetGlobalVars();
		firstPass(file);
		if (!wasError) /* procceed to second pass */
		{ 
			rewind(file);
			secondPass(file, argv[i]);
		}
		printf("\n\n************* Finished %s assembling process *************\n\n", fullFileName);
		free(fullFileName);
		free(mactoName);
	}
	return 0;
}

void resetGlobalVars()
{
	symbolsTable = NULL;
	extList = NULL;
	entryExists = FALSE;
	externExists = FALSE;
	wasError = FALSE;
}


