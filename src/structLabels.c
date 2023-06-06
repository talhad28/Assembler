/*
shahar bar natan - 315638742 || tal hadaya - 318637543
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "utils.h"
#include "ext_vars.h"

/* This function offsets the addresses of a certain group of labels (data/instruction labels)
 * by a given delta (num).
 */
void offsetAddresses(labelPtr label, int num, boolean isData)
{
	while(label)
	{
		/* We don't offset external labels (their address is 0). */
		/* is_data and inActionStatement must have different values in order to meet the same criteria
		* and the XOR operator gives us that */
		if(!(label -> external) && (isData ^ (label -> inActionStatement)))
		{
			label -> address += num;
		}
		label = label -> next;
	}
}

/* This function searches a label in the list and changes his entry field to TRUE and returns TRUE
else if the label doesn't exist return FALSE. */
int makeEntry(labelPtr h, char *name)
{
	labelPtr label = getLabel(h, name);
	if(label != NULL)
	{
		if(label -> external)
		{
			err = ENTRY_CANT_BE_EXTERN;
			return FALSE;
		}
		label -> entry = TRUE;
		entryExists = TRUE; /* Global variable that holds that there was at least one entry in the program */
        return TRUE;
	}
	else
		err = ENTRY_LABEL_DOES_NOT_EXIST;
	return FALSE;
}

/* This function returns the address of a given label, if the label doesn't exist return FALSE (0).*/
unsigned int getLabelAddress(labelPtr h, char *name)
{
	labelPtr label = getLabel(h, name);
	if(label != NULL) 
		return label -> address;
	return FALSE;
}

/* This function check if a label is in the array and an external label is so return 1 else return 0 */
boolean isExternalLabel(labelPtr h, char *name)
{
	labelPtr label = getLabel(h, name);
	if(label != NULL) 
		return label -> external;
	return FALSE;
}

/* This function checks if a given name is a name of a label in the list */
boolean isExistingLabel(labelPtr h, char *name)
{
	return getLabel(h, name) != NULL;
}

/* This function checks if a given label name is in the list if so return 1 else return 0. */
labelPtr getLabel(labelPtr h, char *name)
{
	while(h)
	{
		if(strcmp(h->name,name)==0) /* we found a label with the name given */
			return h;
		h=h->next;
	}
	return NULL;
}

/* This function adds a new label to the linked list of labels given its info. */
labelPtr addLabel(labelPtr *hptr, char *name, unsigned int address, boolean external, ...)
{	
	va_list p;
	
	labelPtr t=*hptr;
	labelPtr temp; /* Auxiliary variable to store the info of the label and add to the list */

	if(isExistingLabel(*hptr, name))
	{
		err = LABEL_ALREADY_EXISTS;
		return NULL;
	}
	temp=(labelPtr) malloc(sizeof(Labels)); 
	if(!temp) /*if we couldn't allocate memory to temp then print an error massage and exit the program*/
	{
		printf("\nerror, cannot allocate memory\n");
		exit(ERROR);
	}

	/* Storing the info of the label in temp */
	strcpy(temp->name, name);
	temp -> entry = FALSE;
	temp -> address = address;
	temp -> external = external;

	if(!external) /* An external label can't be in an action statement */
	{
		va_start(p,external);
		temp -> inActionStatement = va_arg(p,boolean);
	}
	else
	{
		externExists = TRUE;
	}

	/* If the list is empty then we set the head of the list to be temp */
	if(!(*hptr))
	{	
		*hptr = temp;
		temp -> next = NULL;
		return temp;	
	}

	/* Setting a pointer to go over the list until he points on the last label and then stting temp to be the new last label */
	while(t -> next != NULL)
		t = t->next;
	temp -> next = NULL;
	t -> next = temp;

	va_end(p);
	return temp;
}

/* This function frees the allocated memory for the symbols table*/
void freeLabels(labelPtr *hptr)
{
	/* Free the label list by going over each label and free it */
	labelPtr temp;
	while(*hptr)
	{
		temp=*hptr;
		*hptr=(*hptr)->next;
		free(temp);
	}
}

/* This function gets a label's name, searches the list for it and deletes the label.
 * If it managed to delete the label return 1 else return 0
 */
int deleteLabel(labelPtr *hptr, char *name)
{
	/* Goes over the label list and checking if a label by a given name is in the list if it is then deletes it by
	free its space and change the previous label's pointer to point to the next label */
	labelPtr temp = *hptr;
	labelPtr prevtemp;
	while (temp)
	{
		if (strcmp(temp->name, name) == 0) 
		{
			if (strcmp(temp->name, (*hptr)->name) == 0) 
			{
				*hptr = (*hptr)->next;
				free(temp);
			} 
			else 
			{
				prevtemp->next = temp->next;
				free(temp);
			}
			return 1;
		}
		prevtemp = temp;
		temp = temp->next;
	}
	return 0;

}

/* This function prints the list */
void printLabels(labelPtr h)
{
	while (h)
	{
		printf("\nname: %s, address: %d, external: %d", h->name, h->address, h->external);
		if (h->external == 0)
			printf(", is in action statement: %d -> ", h->inActionStatement);
		else
			printf(" -> ");
		h = h->next;
	}
	printf("*");
}
