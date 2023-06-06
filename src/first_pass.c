/*
shahar bar natan - 315638742 || tal hadaya - 318637543
*/

#include "ext_vars.h"
#include "prototypes.h"
#include "utils.h"

/* This function manages all the activities of the first pass */
void firstPass(FILE *fp)
{
	char line[LINE_LENGTH]; /* This string will contain each line at a time */
	int lineNum = 1; /* Line numbers start from 1 */

	ic = 0;
	dc = 0;

	while(fgets(line, LINE_LENGTH, fp) != NULL) /* Read lines until end of file */
	{
	err = NO_ERROR; /* Reset the error global var before parsing each line */
	if(!ignore(line)) /* Ignore line if it's blank or ; */
		readLine(line);
	if(isError())
	{
		wasError = TRUE; /* There was at least one error through all the program */
		writeError(lineNum); /* Output the error */
	}
	lineNum++;
	}

	/* When the first pass ends and the symbols table is complete and IC is evaluated,
	we can calculate real final addresses */
	offsetAddresses(symbolsTable, MEMORY_START, FALSE); /* Instruction symbols will have addresses that start from 100 (MEMORY_START) */
	offsetAddresses(symbolsTable, ic + MEMORY_START, TRUE); /* Data symbols will have addresses that start fron NENORY_START + IC */
}

/* This function will analyze a given line from the file and will extract the information needed by the compiler's rules */
void readLine(char *line)
{
	/* Initializing variables for the type of the directive/command */
	int dirType = UNKNOWN_TYPE;
	int commandType = UNKNOWN_COMMAND;

	boolean label = FALSE; /* This variable will hold TRUE if a label exists in this line */
	labelPtr labelNode = NULL; /* This variable holds optional label in case we create it */
	char currentToken[LINE_LENGTH]; /* This string will hold the current token if we analyze it */

	line = skipSpaces(line); /* skips to the next non-blank/whitepsace character */
	if(endOfLine(line)) return; /* a blank line is not an error */
	if(!isalpha(*line) && *line != '.')
	{ /* first non-blank character must be a letter or a dot */
		err = SYNTAX_ERR;
		return;
	}

	copyToken(currentToken, line); /* Assuming that label is separated from other tokens by a whitespace */
	if(isLabel(currentToken, COLON))
	{ /* We check if the first token is a label (and it should contain a colon) */
		label = TRUE;
		labelNode = addLabel(&symbolsTable, currentToken, 0, FALSE, FALSE); /* adding label to the global symbols table */
		if(labelNode == NULL) /* There was an error creating label */
			return;
		line = nextToken(line); /* Skipping to beginning of next token */
		if(endOfLine(line))
		{
			err = LABEL_ONLY; /* A line can't be label-only */
			return;
		}
		copyToken(currentToken, line); /* Proceed to next token */
	} /* If there's a label error then exit this function */

	if(isError()) /* is_label might return an error */
		return;

	if((dirType = findDirective(currentToken)) != NOT_FOUND) /* detecting directive type (if it's a directive) */
	{
		if(label)
		{
		if(dirType == EXTERN || dirType == ENTRY)
		{ /* we need to ignore creation of label before .entry/.extern */
			deleteLabel(&symbolsTable, labelNode->name);
			label = FALSE;
		}
		else
			labelNode -> address = dc; /* Address of data label is dc */
		}
		line = nextToken(line);
		handleDirective(dirType, line);
	}

	else if ((commandType = findCommand(currentToken)) != NOT_FOUND) /* detecting command type (if it's a command) */
	{
		if(label)
		{
			/* Setting fields accordingly in label */
			labelNode -> inActionStatement = TRUE; 
			labelNode -> address = ic;
		}
		line = nextToken(line);
		handleCommand(commandType, line);
	}

	else
	{
		err = COMMAND_NOT_FOUND; /* a line must have a directive/command */
	}

}

/* This function handles all kinds of directives (.data, .string, .struct, .entry, .extern)
 * and sends them accordingly to the suitable function for analyzing them
 * */
int handleDirective(int type, char *line)
{
	if(line == NULL || endOfLine(line)) /* All directives must have at least one parameter */
	{
		err = DIRECTIVE_NO_PARAMS;
		return ERROR;
	}

	switch (type)
	{
	case DATA:
		/* Handle .data directive and insert values separated by comma to the memory */
		return handleDataDirective(line);

	case STRING:
		/* Handle .string directive and insert all characters (including a '\0') to memory */
		return handleStringDirective(line);

	case STRUCT:
		/* Handle .struct directive and insert both number and string to memory */
		return handleStructDirective(line);

	case ENTRY:
		/* Only check for syntax of entry (should not contain more than one parameter) */
		if(!endOfLine(nextToken(line))) /* If there's a next token (after the first one) */
		{
			err = DIRECTIVE_INVALID_NUM_PARAMS;
			return ERROR;
		}
		break;

	case EXTERN:
		/* Handle .extern directive */
		return handleExternDirective(line);
	}
	return NO_ERROR;
}

/* This function analyzes a command, given the type (mov/cmp/etc...) and the sequence of
 characters starting after the command.
 It will detect the addressing methods of the operands and will encode the first word of
 the command to the instructions memory. */
int handleCommand(int type, char *line)
{
	boolean isFirst = FALSE, isSecond = FALSE; /* These booleans will tell which of the operands were received (not by source/dest, but by order) */
	int firstMethod, secondMethod; /* These will hold the addressing methods of the operands */
	char firstOp[20], secondOp[20]; /* These strings will hold the operands */

	/* Trying to parse 2 operands */
	line = nextListToken(firstOp, line);
	if(!endOfLine(firstOp)) /* If first operand is not empty */
	{
		isFirst = TRUE; /* First operand exists! */
		line = nextListToken(secondOp, line);
	if(!endOfLine(secondOp)) /* If second operand (should hold temporarily a comma) is not empty */
	{
		if(secondOp[0] != ',') /* A comma must separate two operands of a command */
		{
		err = COMMAND_UNEXPECTED_CHAR;
		return ERROR;
		}
		else
		{
			line = nextListToken(secondOp, line); 
			if(endOfLine(secondOp)) /* If second operand is not empty */
			{
				err = COMMAND_UNEXPECTED_CHAR;
				return ERROR;
			}
			isSecond = TRUE; /* Second operand exists! */
		}
	}
	}
	line = skipSpaces(line);
	if(!endOfLine(line)) /* If the line continues after two operands */
	{
		err = COMMAND_TOO_MANY_OPERANDS;
		return ERROR;
	}

	if(isFirst)
		firstMethod = detectMethod(firstOp); /* Detect addressing method of first operand */
	if(isSecond)
		secondMethod = detectMethod(secondOp); /* Detect addressing method of second operand */
	if(!isError()) /* If there was no error while trying to parse addressing methods */
	{
		if(commandAcceptNumOperands(type, isFirst, isSecond)) /* If number of operands is valid for this specific command */
		{
			if(commandAcceptMethods(type, firstMethod, secondMethod)) /* If addressing methods are valid for this specific command */
			{
				/* encode first word of the command to memory and increase ic by the number of additional words */
                encodeToInstructions(buildFirstWord(type, isFirst, isSecond, firstMethod, secondMethod));
				ic += calculateCommandNumAdditionalWords(isFirst, isSecond, firstMethod, secondMethod);
			}

			else
			{
				err = COMMAND_INVALID_OPERANDS_METHODS;
				return ERROR;
			}
		}
		else
		{
			err = COMMAND_INVALID_NUMBER_OF_OPERANDS;
			return ERROR;
		}
	}
	return NO_ERROR;
}


/* This function handles a .string directive by analyzing it and encoding it to data */
int handleStringDirective(char *line)
{
	char token[LINE_LENGTH];

	line = nextTokenString(token, line);
	if(!endOfLine(token) && isString(token)) /* If token exists and it's a valid string */
	{ 
		line = skipSpaces(line);
		if(endOfLine(line)) /* If there's no additional token */
		{
			/* "Cutting" quotation marks and encoding it to data */
			token[strlen(token) - 1] = '\0';
			writeStringToData(token + 1);
		}
		else /* There's another token */
		{
			err = STRING_TOO_MANY_OPERANDS;
			return ERROR;
		}

	}

	else /* Invalid string */
	{
		err = STRING_OPERAND_NOT_VALID;
		return ERROR;
	}
	return NO_ERROR;
}

/* This function handles analyzing and encoding a .struct directive to data memory,
 * given the char sequence starting from after the ".struct"
*/
int handleStructDirective(char *line)
{
	char token[LINE_LENGTH];
	line = nextListToken(token, line); /* Getting the firs token into token array in the line above */

	if(!endOfLine(token) && isNumber(token)) /* First token must be a number */
	{
		writeNumToData(atoi(token)); /* Encode number to data */
		line = nextListToken(token, line); /* Get next token */

		if(!endOfLine(token) && *token == ',')
		{ /* There must be a comma between .struct operands */
			line = nextTokenString(token, line); /* Get next token (second operand) */
			if(!endOfLine(token))
			{ /* There's a second operand */
				if (isString(token))
				{
					/* Encode valid string by "cutting" the "" and sending it to the encoding function */
					token[strlen(token) - 1] = '\0';
					writeStringToData(token + 1);
				}
				else
				{
					err = STRUCT_INVALID_STRING;
					return ERROR;
				}
			}
			else /* There is no second operand */
			{
				err = STRUCT_EXPECTED_STRING;
				return ERROR;
			}
		}
		else
		{
			err = EXPECTED_COMMA_BETWEEN_OPERANDS;
			return ERROR;
		}
	}
	else
	{
		err = STRUCT_INVALID_NUM;
		return ERROR;
	}
	if(!endOfLine(nextListToken(token, line)))
	{
		err = STRUCT_TOO_MANY_OPERANDS;
		return ERROR;
	}
	return NO_ERROR;
}

/* This function parses parameters of a data directive and encodes them to memory */
int handleDataDirective(char *line)
{
	char token[20]; /* Holds tokens */

	/* These booleans mark if there was a number or a comma before current token,
	* so that if there wasn't a number, then a number will be required and
	* if there was a number but not a comma, a comma will be required */
	boolean number = FALSE, comma = FALSE;

	while(!endOfLine(line))
	{
		line = nextListToken(token, line); /* Getting current token */

		if(strlen(token) > 0) /* Not an empty token */
		{
			if (!number)
			{ /* if there wasn't a number before */
				if (!isNumber(token))
				{ /* then the token must be a number */
					err = DATA_EXPECTED_NUM;
					return ERROR;
				}

				else
				{
					number = TRUE; /* A valid number was inputted */
					comma = FALSE; /* Resetting comma (now it is needed) */
					writeNumToData(atoi(token)); /* encoding number to data */
				}
			}

			else if (*token != ',') /* If there was a number, now a comma is needed */
			{
				err = DATA_EXPECTED_COMMA_AFTER_NUM;
				return ERROR;
			}

			else /* If there was a comma, it should be only once (comma should be false) */
			{
				if(comma)
				{
				err = DATA_COMMAS_IN_A_ROW;
				return ERROR;
				}
				else
				{
				comma = TRUE;
				number = FALSE;
				}
			}

		}
	}
	if(comma == TRUE)
	{
		err = DATA_UNEXPECTED_COMMA;
		return ERROR;
	}
	return NO_ERROR;
}

/* This function encodes a given number to data */
void writeNumToData(int num)
{
	data[dc++] = (unsigned int) num;
}

/* This function encodes a given string to data */
void writeStringToData(char *str)
{
	while(!endOfLine(str))
	{
		data[dc++] = (unsigned int) *str; /* Inserting a character to data array */
		str++;
	}
	data[dc++] = '\0'; /* Insert a null character to data */
}

/* This function tries to find the addressing method of a given operand and returns -1 if it was not found */
int detectMethod(char * operand)
{
	char *structField; /* When determining if it's a .struct directive, this will hold the part after the dot */

	if(endOfLine(operand))
		return NOT_FOUND;

	/*----- Immediate addressing method check -----*/
	if (*operand == '#') 
	{ /* First character is '#' */
		operand++;
		if (isNumber(operand))
		return METHOD_IMMEDIATE;
	}

	/*----- Register addressing method check -----*/
	else if (isRegister(operand))
		return METHOD_REGISTER;

	/*----- Direct addressing method check ----- */
	else if (isLabel(operand, FALSE)) /* Checking if it's a label when there shouldn't be a colon (:) at the end */
		return METHOD_DIRECT;

	/*----- Struct addressing method check -----*/
	else if (isLabel(strtok(operand, "."), FALSE))
	{ /* Splitting by dot character */
		structField = strtok(NULL, ""); /* Getting the rest of the string */
		if (strlen(structField) == 1 && (*structField == '1' || /* After the dot there should be '1' or '2' */ *structField == '2'))
			return METHOD_STRUCT;
	}
	err = COMMAND_INVALID_METHOD;
	return NOT_FOUND;
}

/* This function checks for the validity of given addressing methods according to the opcode */
boolean commandAcceptMethods(int type, int firstMethod, int secondMethod)
{
	switch (type)
	{
	/* These opcodes only accept
	 * Source: 0, 1, 2, 3
	 * Destination: 1, 2, 3
	 */
	case MOV:
	case ADD:
	case SUB:
	    return (firstMethod == METHOD_IMMEDIATE ||
		   firstMethod == METHOD_DIRECT ||
		   firstMethod == METHOD_STRUCT ||
		   firstMethod == METHOD_REGISTER)
		&&
		   (secondMethod == METHOD_DIRECT ||
		    secondMethod == METHOD_STRUCT ||
		    secondMethod == METHOD_REGISTER);

	/* LEA opcode only accept
	 * Source: 1, 2
	 * Destination: 1, 2, 3
	*/
	case LEA:
	    return (firstMethod == METHOD_DIRECT ||
		    firstMethod == METHOD_STRUCT)
		   &&
		   (secondMethod == METHOD_DIRECT ||
		    secondMethod == METHOD_STRUCT ||
		    secondMethod == METHOD_REGISTER);

	/* These opcodes only accept
	 * Source: NONE
	 * Destination: 1, 2, 3
	*/
	case NOT:
	case CLR:
	case INC:
	case DEC:
	case JMP:
	case BNE:
	case GET:
	case JSR:
	    return firstMethod == METHOD_DIRECT ||
		   firstMethod == METHOD_STRUCT ||
		   firstMethod == METHOD_REGISTER;

	/* These opcodes are always ok because they accept all methods/none of them and
	 * number of operands is being verified in another function
	*/
	case PRN:
	case CMP:
	case RTS:
	case HLT:
	    return TRUE;
	}

	return FALSE;
	}

	/* This function checks for the validity of given methods according to the opcode */
	boolean commandAcceptNumOperands(int type, boolean first, boolean second)
	{
	switch (type)
	{
	/* These opcodes must receive 2 operands */
	case MOV:
	case CMP:
	case ADD:
	case SUB:
	case LEA:
	    return first && second;

	/* These opcodes must only receive 1 operand */
	case NOT:
	case CLR:
	case INC:
	case DEC:
	case JMP:
	case BNE:
	case GET:
	case PRN:
	case JSR:
	    return first && !second;

	/* These opcodes can't have any operand */
	case RTS:
	case HLT:
	    return !first && !second;
	}
	return FALSE;
}

/* This function calculates number of additional words for a command */
int calculateCommandNumAdditionalWords(int isFirst, int isSecond, int firstMethod, int secondMethod)
{
	int count = 0;
	if(isFirst)
		count += numWords(firstMethod);
	if(isSecond)
		count += numWords(secondMethod);

	/* If both methods are register, they will share the same additional word */
	if(isFirst && isSecond && firstMethod == METHOD_REGISTER && secondMethod == METHOD_REGISTER)
		count--;

	return count;
}

/* This function encodes the first word of the command */
unsigned int buildFirstWord(int type, int isFirst, int isSecond, int firstMethod, int secondMethod)
{
	unsigned int word = 0;

	/* Inserting the opcode */
	word = type;

	word <<= BITS_IN_METHOD; /* Leave space for first addressing method */

	/* If there are two operands, insert the first */
	if(isFirst && isSecond)
		word |= firstMethod;

	word <<= BITS_IN_METHOD; /* Leave space for second addressing method */

	/* If there are two operands, insert the second. */
	if(isFirst && isSecond)
		word |= secondMethod;
	/* If not, insert the first one (a single operand is a destination operand). */
	else if(isFirst)
		word |= firstMethod;

	word = insertAre(word, ABSOLUTE); /* Insert A/R/E mode to the word */

	return word;
}

/* This function returns how many additional words an addressing method requires */
int numWords(int method)
{
	if(method == METHOD_STRUCT) /* Struct addressing method requires two additional words */
		return 2;
	return 1;
}

/* This function handles an .extern directive */
int handleExternDirective(char *line)
{
	char token[LABEL_LENGTH]; /* This will hold the required label */

	copyToken(token, line); /* Getting the next token */
	if(endOfLine(token)) /* If the token is empty, then there's no label */
	{
		err = EXTERN_NO_LABEL;
		return ERROR;
	}
	if(!isLabel(token, FALSE)) /* The token should be a label (without a colon) */
	{
		err = EXTERN_INVALID_LABEL;
		return ERROR;  
	}  

	line = nextToken(line);
	if(!endOfLine(line))
	{
		err = EXTERN_TOO_MANY_OPERANDS;
		return ERROR;
	}

	/* Trying to add the label to the symbols table */
	if(addLabel(&symbolsTable, token, EXTERNAL_DEFAULT_ADDRESS, TRUE) == NULL)
		return ERROR;
	return isError(); /* Error code might be 1 if there was an error in is_label() */
}

/* This function checks whether a given token is a label or not (by syntax).
 * The parameter colon states whether the function should look for a ':' or not
 * when parsing parameter (to make it easier for both kinds of tokens passed to this function.
 */
boolean isLabel(char *token, int colon)
{
	boolean hasDigits = FALSE; /* If there are digits inside the label, we can easily skip checking if
		                   it's a command name. */
	int tokenLen = strlen(token);
	int i;

	/* Checking if token's length is not too short */
	if(token == NULL || tokenLen < (colon ? MINIMUM_LABEL_LENGTH_WITH_COLON: MINIMUM_LABEL_LENGTH_WITHOUT_COLON))
		return FALSE;

	if(colon && token[tokenLen - 1] != ':')
		return FALSE; /* if colon = TRUE, there must be a colon at the end */

	if (tokenLen > LABEL_LENGTH) 
	{
		if(colon) 
			err = LABEL_TOO_LONG; /* It's an error only if we search for a label definition */
		return FALSE;
	}
	if(!isalpha(*token))
	{ /* First character must be a letter */
		if(colon)
			err = LABEL_INVALID_FIRST_CHAR;
		return FALSE;
	}

	if (colon) 
	{
		token[tokenLen - 1] = '\0'; /* The following part is more convenient without a colon */
		tokenLen--;
	}

	/* Check if all characters are digits or letters */
	for(i = 1; i < tokenLen; i++) /* We have already checked if the first character is ok */
	{
		if(isdigit(token[i]))
			hasDigits = TRUE;
		else if(!isalpha(token[i])) 
		{
			/* It's not a label but it's an error only if someone put a colon at the end of the token */
			if(colon)
				err = LABEL_ONLY_ALPHANUMERIC;
			return FALSE;
		}
	}

	if(!hasDigits) /* It can't be a command */
	{
		if (findCommand(token) != NOT_FOUND) {
			if(colon)
				err = LABEL_CANT_BE_COMMAND; /* Label can't have the same name as a command */
			return FALSE;
		}
	}

	if(isRegister(token)) /* Final obstacle: it's a label only if it's not a register */
	{
		if(colon)
			err = LABEL_CANT_BE_REGISTER;
		return FALSE;
	}

	return TRUE;
}
