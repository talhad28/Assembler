/*
shahar bar natan - 315638742 || tal hadaya - 318637543
*/

#include "macro.h"
/*We implement a table of macros*/

int IsMacroOrEndm(char line[]) /*Checks whether it is the beginning of a macro or the end of a macro*/
{
  int index = 0 , mindex = 0 ;
  char macro [MAX];
  memset(macro , '\0' , MAX);
  while(isspace(line[index]))
       index ++;
  while (!isspace(line[index]) && line[index] != '\n')
  {
    macro[mindex] = line[index];
    mindex++;
    index++;
  }
  if (!strcmp(macro, "macro"))
    return 1;
    if (!strcmp(macro, "endmacro"))
    return 2;
  return 0;
}


void InsertName(struct Macro *temp, char line[]) /*Enter the macro name in the macros table*/
{
  int index = 0 , nindex = 0 ;
  char name [MAX];
  memset(name , '\0' , MAX);
  while(isspace(line[index]))
    index ++;
  while (!isspace(line[index]) && line[index] != '\n')
    index++;
    while(isspace(line[index]))
    index ++;
  while (!isspace(line[index]) && line[index] != '\n')
  {
    name[nindex] = line[index];
    nindex++;
    index++;
  }
  strcpy(temp->mname,name);
}


void InsertContent(struct Macro *temp, FILE *fp) /*Inserts the macro contents into the macros table*/
{
  char line [MAX];
  char content [MAX];
  memset(line , '\0' , MAX);
  memset(content , '\0' , MAX);
  fgets(line, MAX, fp);
 while(IsMacroOrEndm(line) != 2)
  {
    strncat(content, line , MAX);   
    fgets(line, MAX, fp);
  }
  strcpy(temp->mcontent,content);
}


int PreReadFile(char *file,struct Macro *head) /*Performing the first pass on the file (inserting the macros into the macro table,
                                                        copying the corresponding rows from the table to the file, etc.)*/
{
  char line [MAX];
  FILE *fp;
  struct  Macro* temp;
  memset(line , '\0' , MAX);
  fp = fopen(file,"r");
  if(fp == NULL)
  {
     printf("error: cant open the file: %s \n \n" , file);  
     return 1;
  }
    while(fgets(line, MAX, fp))
  {
   if(IsMacroOrEndm(line) == 1)
      { 
        temp = NULL;
        temp = (struct Macro*)malloc(sizeof(struct Macro));
        InsertName(temp , line);
        InsertContent(temp , fp);
        head -> next = temp ;
        head = temp;
      }
  }
  close(fp);
  return 0;
}


int IsMacroCall(char line[], FILE *fpw,struct Macro *tail) /*Copy the contents of the corresponding macro to the file from the table, if it is a macro command*/
{
  int index = 0, mindex = 0;
  char Mname [MAX];
  struct  Macro *temp = NULL;
  temp = (struct Macro*)malloc(sizeof(struct Macro));
  temp = tail;
  memset(Mname, '\0' , MAX);
  while(isspace(line[index]))
    index ++;
  while (!isspace(line[index]) && line[index] != '\n')
  {
    Mname[mindex] = line[index];
    mindex++;
    index++;
  }
  while (temp != NULL)
  {
    if (!strcmp(temp->mname , Mname))
    {
    fprintf(fpw, "%s", temp->mcontent);
    return 1;
    }
    temp = temp ->next;
  }
  return 0;
}


void PreWriteFile(char *asFileName, char *amFileName, struct Macro *tail)
{
  int macroflag = 0; 
  FILE *fpw, *fpr;
  char line [MAX];
  memset(line , '\0' , MAX);
  fpr = fopen(asFileName,"r");
  fpw = fopen(amFileName,  "w");
  if(fpr == NULL)
     printf("error: cant open the file: %s \n \n" , asFileName); 
  while(fgets(line, MAX, fpr))
  {
    if(!macroflag)
    {   
       if(!IsMacroCall(line, fpw, tail))
       {
         if(IsMacroOrEndm(line) == 0)
         {
            fprintf(fpw,"%s",line);
         }
         else 
         {
            macroflag = 1;
         }
       }
    }
    else
    {
      if(IsMacroOrEndm(line) == 2)
        macroflag = 0;
    }        
  } 
  fclose(fpw);
  fclose(fpr);
}
