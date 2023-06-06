/*
shahar bar natan - 315638742 || tal hadaya - 318637543
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX 80

struct Macro {
    char mname[MAX];
    char mcontent[MAX];
    struct Macro* next;
};
void addToMtable(struct Macro *head , char name[] , char content[]);
void printMacroTable(struct Macro *tail);
int IsMacroOrEndm(char line[]);
void InsertName(struct Macro *temp, char line[]);
void InsertContent(struct Macro *temp, FILE *fp);
int PreReadFile(char *file,struct Macro *head);
int IsMacroCall(char line[], FILE *fpw,struct Macro *tail);
void PreWriteFile(char *asFileName, char *amFileName, struct Macro *tail);
