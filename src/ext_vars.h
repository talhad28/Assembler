/*
shahar bar natan - 315638742 || tal hadaya - 318637543
*/

#include "structs.h"
#include "assembler.h"


extern int ic, dc;
extern int err;
extern boolean wasError;
extern labelPtr symbolsTable;
extern extPtr extList;
extern const char base32[32];
extern const char *commands[];
extern const char *directives[];
extern boolean entryExists, externExists;
