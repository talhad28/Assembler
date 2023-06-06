/*
shahar bar natan - 315638742 || tal hadaya - 318637543
*/

#include "ext_vars.h"
#include "prototypes.h"
#include "utils.h"

void secondPass(FILE *fp, char *filename)
{
	char line[LINE_LENGTH]; /* This string will contain each line at a time */
	int lineNum = 1; /* Line numbers start from 1 */

	ic = 0; /* Initializing the global instructions counter */

	while(fgets(line, LINE_LENGTH, fp) != NULL) /* Read lines until end of file */
	{
		err = NO_ERROR;
		if(!ignore(line)) /* Ignore line if it's blank or ; */
			readLineSecondPass(line); /* Analyze one line at a time */
		if(isError())/* If there was an error in the current line */
		{ 
			wasError = TRUE; /* There was at least one error through all the program */
			writeError(lineNum);
		}
		lineNum++;
	}
	if(!wasError) /* Write output files only if there weren't any errors in the program */
		writeOutputFiles(filename);

	/* Free dynamic allocated elements */
	freeLabels(&symbolsTable);
	freeExt(&extList);
}

/* This function analyzes and extracts information needed for the second pass from a given line */
void readLineSecondPass(char *line)
{
	int dirType, commandType;
	char currentToken[LINE_LENGTH]; /* will hold current token as needed */
	line = skipSpaces(line); /* Proceeding to first non-blank character */
	if(endOfLine(line))/* a blank line is not an error */
		return; 
	copyToken(currentToken, line);
	if(isLabel(currentToken, COLON))/* If it's a label, skip it */
	{ 
		line = nextToken(line);
		copyToken(currentToken, line);
	}

	if((dirType = findDirective(currentToken)) != NOT_FOUND) /* We need to handle only .entry directive */
	{
		line = nextToken(line);
		if(dirType == ENTRY)
		{
			copyToken(currentToken, line);
			makeEntry(symbolsTable, currentToken); /* Creating an entry for the symbol */
		}
	}

	else if ((commandType = findCommand(currentToken)) != NOT_FOUND) /* Encoding command's additional words */
	{
		line = nextToken(line);
		handleCommandSecondPass(commandType, line);
	}
}

/* This function writes all 3 output files (if they should be created)*/
int writeOutputFiles(char *original)
{
	FILE *file;

	file = openFile(original, FILE_OBJECT);
	writeOutputOb(file);

	if(entryExists)
	{
		file = openFile(original, FILE_ENTRY);
		writeOutputEntry(file);
	}

	if(externExists)
	{
		file = openFile(original, FILE_EXTERN);
		writeOutputExtern(file);
	}

	return NO_ERROR;
}

/* This function writes the .ob file output.
 * The first line is the size of each memory (instructions and data).
 * Rest of the lines are: address in the first column, word in memory in the second.
 */
void writeOutputOb(FILE *fp)
{
	unsigned int address = MEMORY_START;
	int i;
	char *param1 = convertToBase32(ic), *param2 = convertToBase32(dc);

	fprintf(fp, "%s\t%s\n\n", param1, param2); /* First line */
	free(param1);
	free(param2);

	for (i = 0; i < ic; address++, i++) /* Instructions memory */
	{
		param1 = convertToBase32(address);
		param2 = convertToBase32(instructions[i]);

		fprintf(fp, "%s\t%s\n", param1, param2);

		free(param1);
		free(param2);
	}

	for (i = 0; i < dc; address++, i++) /* Data memory */
	{
		param1 = convertToBase32(address);
		param2 = convertToBase32(data[i]);

		fprintf(fp, "%s\t%s\n", param1, param2);

		free(param1);
		free(param2);
	}

	fclose(fp);
}

/* This function writes the output of the .ent file.
 * First column: name of label.
 * Second column: address of definition.
 */
void writeOutputEntry(FILE *fp)
{
	char *base32Address;

	labelPtr label = symbolsTable;
	/* Go through symbols table and print only symbols that have an entry */
	while(label)
	{
		if(label -> entry)
		{
			base32Address = convertToBase32(label -> address);
			fprintf(fp, "%s\t%s\n", label -> name, base32Address);
			free(base32Address);
		}
		label = label -> next;
	}
	fclose(fp);
}

/* This function writes the output of the .ext file.
 * First column: label name.
 * Second column: address where the external label should be replaced.
 */
void writeOutputExtern(FILE *fp)
{
	char *base32Address;
	extPtr node = extList;

	/* Going through external circular linked list and pulling out values */
	do
	{
		base32Address = convertToBase32(node -> address);
		fprintf(fp, "%s\t%s\n", node -> name, base32Address); /* Printing to file */
		free(base32Address);
		node = node -> next;
	} while(node != extList);
	fclose(fp);
}

/* This function opens a file with writing permissions, given the original input filename and the
 * wanted file extension (by type)
 */
FILE *openFile(char *filename, int type)
{
	FILE *file;
	filename = createFileName(filename, type); /* Creating filename with extension */

	file = fopen(filename, "w"); /* Opening file with permissions */
	free(filename); /* Allocated modified filename is no longer needed */

	if(file == NULL)
	{
		err = CANNOT_OPEN_FILE;
		return NULL;
	}
	return file;
}

/* This function determines if source and destination operands exist by opcode */
void checkOperandsExist(int type, boolean *isSource, boolean *isDest)
{
	switch (type)
	{
		case MOV:
		case CMP:
		case ADD:
		case SUB:
		case LEA:
			*isSource = TRUE;
			*isDest = TRUE;
			break;

		case NOT:
		case CLR:
		case INC:
		case DEC:
		case JMP:
		case BNE:
		case GET:
		case PRN:
		case JSR:
			*isSource = FALSE;
			*isDest = TRUE;
			break;

		case RTS:
		case HLT:
			*isSource = FALSE;
			*isDest = FALSE;
	}
}

/* This function handles commands for the second pass - encoding additional words */
int handleCommandSecondPass(int type, char *line)
{
	char firstOp[LINE_LENGTH], secondOp[LINE_LENGTH]; /* will hold first and second operands */
	char *src = firstOp, *dest = secondOp; /* after the check below, src will point to source and dest to destination operands */
	boolean isSource = FALSE, isDest = FALSE; /* Source/destination operands existence */
	int src_method = METHOD_UNKNOWN, dest_method = METHOD_UNKNOWN; /* Their addressing methods */

	checkOperandsExist(type, &isSource, &isDest);

	/* Extracting source and destination addressing methods */
	if(isSource)
		src_method = extractBits(instructions[ic], SRC_METHOD_START_POS, SRC_METHOD_END_POS);
	if(isDest)
		dest_method = extractBits(instructions[ic], DEST_METHOD_START_POS, DEST_METHOD_END_POS);

	/* Matching src and dest pointers to the correct operands (first or second or both) */
	if(isSource || isDest)
	{
		line = nextListToken(firstOp, line);
		if(isSource && isDest) /* There are 2 operands */
		{
			line = nextListToken(secondOp, line);
			nextListToken(secondOp, line);
		}
		else
		{
			dest = firstOp; /* If there's only one operand, it's a destination operand */
			src = NULL;
		}
	}

	ic++; /* The first word of the command was already encoded in this IC in the first pass */
	return encodeAdditionalWords(src, dest, isSource, isDest, src_method, dest_method);
}

/* This function encodes the additional words of the operands to instructions memory */
int encodeAdditionalWords(char *src, char *dest, boolean isSource, boolean isDest, int src_method, int dest_method)
{
	/* There's a special case where 2 register operands share the same additional word */
	if(isSource && isDest && src_method == METHOD_REGISTER && dest_method == METHOD_REGISTER)
	{
		encodeToInstructions(buildRegisterWord(FALSE, src) | buildRegisterWord(TRUE, dest));
	}
	else /* It's not the special case */
	{
		if(isSource)
			encodeAdditionalWord(FALSE, src_method, src);
		if(isDest)
			encodeAdditionalWord(TRUE, dest_method, dest);
	}
	return isError();
}

/* This function builds the additional word for a register operand */
unsigned int buildRegisterWord(boolean isDest, char *reg)
{
	unsigned int word = (unsigned int) atoi(reg + 1); /* Getting the register's number */
	/* Inserting it to the required bits (by source or destination operand) */
	if(!isDest)
		word <<= BITS_IN_REGISTER;
	word = insertAre(word, ABSOLUTE);
	return word;
}

/* This function encodes a given label (by name) to memory */
void encodeLabel(char *label)
{
	unsigned int word; /* The word to be encoded */

	if(isExistingLabel(symbolsTable, label))
	{ /* If label exists */
		word = getLabelAddress(symbolsTable, label); /* Getting label's address */

		if(isExternalLabel(symbolsTable, label))
		{ /* If the label is an external one */
			/* Adding external label to external list (value should be replaced in this address) */
			addExt(&extList, label, ic + MEMORY_START);
			word = insertAre(word, EXTERNAL);
		}
		else
		word = insertAre(word, RELOCATABLE); /* If it's not an external label, then it's relocatable */
		encodeToInstructions(word); /* Encode word to memory */
	}
	else /* It's an error */
	{
		ic++;
		err = COMMAND_LABEL_DOES_NOT_EXIST;
	}
}

/* This function encodes an additional word to instructions memory, given the addressing method */
void encodeAdditionalWord(boolean isDest, int method, char *operand)
{
	unsigned int word = EMPTY_WORD; /* An empty word */
	char *temp;

	switch (method)
	{
	case METHOD_IMMEDIATE: /* Extracting immediate number */
		word = (unsigned int) atoi(operand + 1);
		word = insertAre(word, ABSOLUTE);
		encodeToInstructions(word);
		break;

	case METHOD_DIRECT:
		encodeLabel(operand);
		break;

	case METHOD_STRUCT: /* Before the dot there should be a label, and after it a number */
		temp = strchr(operand, '.');
		*temp = '\0';
		encodeLabel(operand); /* Label before dot is the first additional word */
		*temp++ = '.';
		word = (unsigned int) atoi(temp);
		word = insertAre(word, ABSOLUTE);
		encodeToInstructions(word); /* The number after the dot is the second */
	break;

	case METHOD_REGISTER:
		word = buildRegisterWord(isDest, operand);
		encodeToInstructions(word);
	}
}
