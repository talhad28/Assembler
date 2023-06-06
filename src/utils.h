/*
shahar bar natan - 315638742 || tal hadaya - 318637543
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include "prototypes.h"
#include "assembler.h"
#include "ext_vars.h"
#include "structs.h"

char* asFile(const char*); /*the function add ".as" to the end of the file name*/
char* amFile(const char*); /*the function add ".am" to the end of the file name*/

/* Functions that are used for parsing tokens and navigating through them */
char *nextTokenString(char*, char*);
char *nextListToken(char*, char*);
char *nextToken(char*);
char *skipSpaces(char*);
void copyToken(char*, char*);
int endOfLine(char*);
int ignore(char*);

/* Functions that are used to determine types of tokens */
int findIndex(char *, const char *arr[], int);
int findCommand(char *);
int findDirective(char *);
boolean isString(char *);
boolean isNumber(char *);
boolean isRegister(char *);

/* Functions that are used for creating files and assigning required extensions to them */
char *createFileName(char *, int );
FILE *openFile(char *, int );
char *convertToBase32(unsigned int );

/* Functions of external labels positions' linked list */
extPtr addExt(extPtr *, char *, unsigned int );
void freeExt(extPtr *);
void printExt(extPtr );

/* Functions of symbols table */
labelPtr addLabel(labelPtr *, char *, unsigned int , boolean external, ...);
int deleteLabel(labelPtr *, char *);
void freeLabels(labelPtr *);
void offsetAddresses(labelPtr , int , boolean );
unsigned int getLabelAddress(labelPtr , char *);
labelPtr getLabel(labelPtr , char *);
boolean isExistingLabel(labelPtr , char *);
boolean isExternalLabel(labelPtr , char *);
int makeEntry(labelPtr,  char*);
void printLabels(labelPtr );

/* Functions that handle errors */
void writeError(int ); /* This function is called when an error output is needed */
int isError();

/* Functions for encoding and building words */
unsigned int extractBits(unsigned int , int , int );
void encodeToInstructions(unsigned int );
unsigned int insertAre(unsigned int , int );
