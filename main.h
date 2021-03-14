#include "errors.h"
#include "bool.h"
#include "dsnodes.h"

#define MAKE_OBJ_NAME_STRING \
    strcpy(path, argv[argc]); \
    strcat(path, ".ob")

/*the first_phase function goes through the assembly code, builds symbol table,
computes the number of words needed for the data and code in the output files
and checks for some errors.
returns if any errors where found or not*/
int first_phase(FILE *fd, char *path, snode **symbhead, dnode **dathead, int *, int *);

int second_phase(FILE *fd, char *path, snode **symbhead, dnode **inshead, snode **);
void make_files(snode *, snode *symbhead, dnode *dathead, dnode *inshead, char *exfilename, int inslines, int datlines);