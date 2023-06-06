/*
shahar bar natan - 315638742 || tal hadaya - 318637543
*/

#ifndef PROTOTYPES_H
#define PROTOTYPES_H
#include <stdio.h>
#include "structs.h"

void firstPass(FILE *fp);
void secondPass(FILE *fp, char *filename);

/*------------------ First Pass Functions ------------------*/

unsigned int buildFirstWord(int, int, int, int, int);
int calculateCommandNumAdditionalWords(int, int, int, int);
boolean commandAcceptMethods(int, int, int);
boolean commandAcceptNumOperands(int , boolean, boolean);
int detectMethod(char*);
int handleCommand(int , char*);
int handleDataDirective(char*);
int handleDirective(int type, char*);
int handleExternDirective(char*);
int handleStringDirective(char*);
int handleStructDirective(char*);
boolean isLabel(char*, int );
int numWords(int );
void readLine(char*);
void writeNumToData(int );
void writeStringToData(char*);

/*------------------ Second Pass Functions ------------------*/

unsigned int buildRegisterWord(boolean is_dest, char *reg);
void checkOperandsExist(int type, boolean *is_src, boolean *is_dest);
int encodeAdditionalWords(char *src, char *dest, boolean is_src, boolean is_dest, int src_method, int dest_method);
void encodeAdditionalWord(boolean is_dest, int method, char *operand);
void encode_label(char *label);
int handleCommandSecondPass(int type, char *line);
void readLineSecondPass(char *line);
void writeOutputEntry(FILE *fp);
void writeOutputExtern(FILE *fp);
int writeOutputFiles(char *original);
void writeOutputOb(FILE *fp);

#endif
