#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "errors.h"
#include "analyzeline.h"
#include "bool.h"
#include "dsnodes.h"

#define ARE_A 0
#define ARE_R 2
#define ARE_E 1

#define ORIGREGP 5
#define DESTREGP 2

#define SP_0 '*'
#define SP_1 '#'
#define SP_2 '%'
#define SP_3 '!'

/*check if array is external*/
#define CHECK_EXTERN(varname, a_r_e, extlist, oper, linectr) \
{ \
    /*checks if the variable is external*/ \
    if(varname -> type == symtype_extern) \
    { \
        a_r_e = ARE_E; \
        /*add the variabe to the ext list*/ \
        add_sym_node(extlist, oper, (linectr), 0, 0); \
    } \
    else \
        a_r_e = ARE_R; \
}

/*check if array is external and make code the words*/
#define CHECK_EXT_AND_UPDATE_INDX(varname, a_r_e, extlist, oper, linectr, head, indexword) \
{ \
            /*checks if the array is external*/ \
            CHECK_EXTERN(varnode, a_r_e, extlist, oper, (linectr)) \
            /*creates the word in decimeal*/ \
            arrnameword = ((varnode -> value) << 2) | a_r_e; \
            /*add the array to the commands list*/ \
            add_dat_node(head, arrnameword); \
            /*add the index to the commands list*/ \
            add_dat_node(head, indexword << 2); \
            /*update line counter*/ \
            (linectr) += 2; \
}

/*prints a line to the '.ob' file, moves the pointer to the next node and updates line counter*/
#define PRINT_TO_OB_AND_NEXT(fd, linectr, node) \
    fprintf(fd, "%d\t",linectr); \
    to_special_quaternary(node  -> value, fd); \
    linectr++; \
    node = node -> next;

/*checks if index of an array is out of bounds */
int out_of_array_bounds(snode *, int);
/*codes an immediate operand. returns TRUE if code is good so far*/
int code_imdt(char *, int , char *, dnode **, snode **, int *);
/*codes a direct operand. returns TRUE if code is good so far*/
int code_direct(snode **, dnode **, snode **, char *, int , char *, int *);
/*codes a regiser operand. we dont need to check if this part is ok. the variable places indicates if the operand is origin or destination*/
void code_reg(dnode **, char *, int , int *);
/*codes an array type operand. returns TRUE if code is good so far.
this function also checks if the array name is in the symbol list and if the value of the index is OK and in range*/
int code_indxed(snode **, dnode **, snode **, char *, int , char *, int *);
/*creates the files*/
void make_files(snode *, snode *, dnode *, dnode *, char *, int , int );
/*creates '.ob' file */
void make_obj_file(dnode *, char *, int , int , dnode *, char *);
/*creates '.ent' and '.ext' files */
void make_ent_file(snode *, char *);
/*creates '.ext' and '.ext' files */
void make_ext_file(snode *, char *);
/*converts an int to character array which represents our PC special base 4 system*/
void to_special_quaternary(unsigned, FILE *);
