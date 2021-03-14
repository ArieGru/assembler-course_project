#include "codeandbuild.h"

/*checks if index of an array is out of bounds */
int out_of_array_bounds(snode *node, int number)
{
    /*if the symbol is external it can be in array bounds*/
    if(node -> type == symtype_extern)
        return FALSE;
    /*index is out of bounds if is bigger than the array size or smaller than 0*/
    if(number < 0 || node -> numofwords <= 0 || number >= node -> numofwords)
        return TRUE;
    else
        return FALSE;
}
/*builds the list of commands into the structure 'head'. the structure will contain the code lines in the order they should be.
    the function also creates the list to the 'ext' file in the structure 'extlist' if there are any lines that should go there.
    returns TRUE if succeded.*/
int codeinstructions(snode **extlist, dnode **head, snode **symbhead, int numofwords, enum commands com, char *origop, char *destop,
                     enum address_type origadd, enum address_type destadd, int linenumber, char *path, int *linectr)
{
    int codeisok = TRUE;
    int firstword;
    /*build first word */
    if(origadd == empty_address && destadd == empty_address)    /*in case we have a command without any operands*/
        firstword = (com - 1) << 6;
    else if(origadd == empty_address)
        firstword = (com - 1) << 6 | destadd << 2;  /*in case we have a command with only one operand. no valid command gets only origin operand, so we ignored this situation*/
    else
        firstword = (com - 1) << 6 | origadd << 4 | destadd << 2;   /*in case we have a command with two operands*/
    /*add first word to list*/
    add_dat_node(head, firstword);
    (*linectr)++;
    /*in case both operands are registers*/
    if(origop != NULL && destop != NULL && origadd == regis && destadd == regis)
    {
        int origregnum = atoi(++origop);    /*origop points to the letter 'r', so we will ignore it*/
        int destregnum = atoi(++destop);    /*destop points to the letter 'r', so we will ignore it*/
        int codeword = (origregnum << 5) | (destregnum << 2);
        add_dat_node(head, codeword);
        (*linectr)++;
    }
    else
    {
        /*in case we have an origin operand*/
        if(origadd != empty_address)
        {
            if(origadd == imdt)
                codeisok = (code_imdt(origop, linenumber, path, head, symbhead, linectr) && codeisok);
            else if(origadd == direct)
                codeisok = (code_direct(extlist, head, symbhead, origop, linenumber, path, linectr) && codeisok);
            else if(origadd == regis)
               code_reg(head, origop, ORIGREGP, linectr);
            /*else operand must be array index */
            else
                codeisok = (code_indxed(extlist, head, symbhead, origop, linenumber, path, linectr) && codeisok);
        }  
        /*in case we have a destination operand*/
        if(destadd != empty_address)
        {
            if(destadd == imdt)
                codeisok = (code_imdt(destop, linenumber, path, head, symbhead, linectr) && codeisok);
            else if(destadd == direct)
                codeisok = (code_direct(extlist, head, symbhead, destop, linenumber, path, linectr) && codeisok);
            else if(destadd == regis)
                code_reg(head, destop, DESTREGP, linectr);
            /*else operand must be array index */
            else
                codeisok = (code_indxed(extlist, head, symbhead, destop, linenumber, path, linectr) && codeisok);
        }
    }
    /*in case some unknown error occured*/
    if(origadd == empty_address && destadd == empty_address && com != stop && com != rts)
        ERROR_MESSAGE(codeisok, linenumber, path, "unknown error occured.")

    /*if an error detected in this function we will return FALSE. otherwise  we return TRUE*/
    return codeisok;
}

/*codes an immediate operand. returns TRUE if code is good so far*/
int code_imdt(char *oper, int linenumber, char *path, dnode **head, snode **symbhead, int *linectr)
{
    int codeword;
    snode *macnode; 
    /*first character is '#'. check if the operand is a number*/ 
    if(number_is_legit(oper + 1)) \
    { 
        /*chekcks if the character after '#' is not a number.*/ 
        if(!isdigit(*(oper + 1)) && *(oper + 1) != '-' && *(oper + 1) != '+') 
        {  
            printf("Error in line %d in file \"%s\": there must be a valid integer right afrter '#' sign.\n", linenumber, path); 
            return FALSE;
        } 
        else
        {
            codeword = (atoi(oper + 1) << 2); 
            add_dat_node(head, codeword);
            (*linectr)++;
        }
    } 
    else 
    { 
        if((macnode = macro_is_in_symtable(*symbhead, oper + 1, TRUE)) == NULL) 
        { 
            printf("Error in line %d in file \"%s\": macro name does not exist.\n", linenumber, path); 
            return FALSE;
        } 
        else 
        { 
            codeword = (macnode -> value) << 2; 
            add_dat_node(head, codeword); 
            (*linectr)++;
        } 
    }
    /*we will rget here only if no error was detected*/
    return TRUE;
}

/*codes a direct operand. returns TRUE if code is good so far*/
int code_direct(snode **extlist, dnode **head, snode **symbhead, char *oper, int linenumber, char *path, int *linectr) 
{ 
    int codeword; 
    snode *labnode; /*stores the node in symbol table with the name of the macro or label*/
    /*check if the label or macro name isn't in the symbol table*/
    if((labnode = macro_is_in_symtable(*symbhead, oper, FALSE)) == NULL) 
    { 
        printf("Error in line %d in file \"%s\": macro name does not exist.\n", linenumber, path); 
        return FALSE;
    } 
    else 
    { 
        int a_r_e;
        CHECK_EXTERN(labnode, a_r_e, extlist, oper, (*linectr))
        /*creates the word in decimeal*/
        codeword = ((labnode -> value) << 2) | a_r_e; 
        /*add the variable to the commands list*/
        add_dat_node(head, codeword); 
        /*update line counter*/
        (*linectr)++;
    } 
    /*we will rget here only if no error was detected*/
    return TRUE;
}
/*codes a regiser operand. we dont need to check if this part is ok. the variable places indicates if the operand is origin or destination*/
void code_reg(dnode **head, char *oper, int places, int *linectr) 
{
    /*creates the word in decimeal*/
    int codeword = (atoi(++oper)) << places;
    /*add the variable to the commands list*/
    add_dat_node(head, codeword);
    /*update line counter*/
    (*linectr)++;
}
/*codes an array type operand. returns TRUE if code is good so far.
this function also checks if the array name is in the symbol list and if the value of the index is OK and in range*/
int code_indxed(snode **extlist, dnode **head, snode **symbhead, char *oper, int linenumber, char *path, int *linectr) 
{ 
    char *varname = strtok(oper, "["); /*stores the array name*/
    char *indname = strtok(NULL, "]"); /*stores the index name or number*/
    snode *varnode = macro_is_in_symtable(*symbhead, varname, FALSE); /*stores the node in symbol table with the array name. if NULL the array name isn't in the symbol table */
    snode *indnode;     /*stores the index node in the symbol table if the index isn't a number*/
    /*in case the array isn't in the symbol table name*/
    if(varnode == NULL) 
    { 
        printf("Error in line %d in file \"%s\": array name does not exist.\n", linenumber, path); 
        return FALSE;
    }
    /*if the index name isn't a number*/
    else if(!number_is_legit(indname)) 
    { 
        /*checks if the index name is in the symbol table*/
        if((indnode = macro_is_in_symtable(*symbhead, indname, FALSE)) == NULL) 
        { 
            printf("Error in line %d in file \"%s\": index name does not exist in the code.\n", linenumber, path); 
            return FALSE;
        }
        /*checks if the index is out of array bounds*/
        else if(out_of_array_bounds(varnode, indnode -> value)) 
        { 
            printf("Error in line %d in file \"%s\": index out of array bounds.\n", linenumber, path); 
            return FALSE;
        }
        else 
        { 
            int arrnameword; 
            int arrindxword = (indnode -> value) << 2;
            int a_r_e;
            /*check if array is external and make code the words*/
            CHECK_EXT_AND_UPDATE_INDX(varname, a_r_e, extlist, oper, *linectr, head, arrindxword)
         } 
    } 
    /*in case index name is a number*/ 
    else 
    { 
        int indexnum = atoi(indname); /*index in decimal*/
        /*checks if the index is out of array bounds*/
        if(out_of_array_bounds(varnode, indexnum)) 
        { 
            printf("Error in line %d in file \"%s\": index out of array bounds.\n", linenumber, path); 
            return FALSE;
        }  
        else 
        { 
            int arrnameword; 
            int a_r_e;
            /*check if array is external and make code the words*/
            CHECK_EXT_AND_UPDATE_INDX(varname, a_r_e, extlist, oper, *linectr, head, indexnum)
        } 
    } 
    /*we will rget here only if no error was detected*/
    return TRUE;
}
/*creates the files*/
void make_files(snode *extlist, snode *symbhead, dnode *dathead, dnode *inshead, char *filename, int inslines, int datlines)
{
    char obfilename[MAX_PATH_LENGTH], entfilename[MAX_PATH_LENGTH], extfilename[MAX_PATH_LENGTH];
    /*creates string with path name with '.ob' extension */
    strcpy(obfilename, filename); 
    strcat(obfilename, ".ob");
    /*creates string with path name with '.ent' extension */
    strcpy(entfilename, filename); 
    strcat(entfilename, ".ent");
    /*creates string with path name with '.ext' extension */
    strcpy(extfilename, filename); 
    strcat(extfilename, ".ext");

    /*make '.ob' file if there is data*/
    make_obj_file(inshead, obfilename, inslines, datlines, dathead, extfilename);
    /*make '.ent' file if there is data*/
    make_ent_file(symbhead, entfilename);
    /*make '.ext' file if there is data*/
    make_ext_file(extlist, extfilename);
}
/*creates '.ob' file */
void make_obj_file(dnode *dhead, char *path, int inslines, int datlines, dnode *shead, char *extfilename)
{
    int linectr = 100; /*count ilnes. the code starts from 100 */
    FILE *fd = NULL;    /*pointer to the output file*/
    /*checks if the list of code data is empty*/
    if(dhead != NULL)
    {
        dnode *node = dhead;
        fd = fopen(path, "w");  /*create the file*/
        fprintf(fd, "  %d  %d\n",inslines, datlines);   /*make 'header' to the file*/
        /*print first line to the file and moves pointer*/
        
        do
        {
            /*print line to the file and moves pointer*/
            PRINT_TO_OB_AND_NEXT(fd, linectr, node)
        }
        while (node != dhead);

    }
    /*prints the data part */
    if(shead != NULL)
    {
        dnode *node = shead ;
        /*in case there is no code and only data, create the file */
        if(fd == NULL)
        {
            fd = fopen(path, "w");
            fprintf(fd, "  %d  %d\n",inslines, datlines);
        }

        /*print line to the file and moves pointer*/
        
        do
        {
            PRINT_TO_OB_AND_NEXT(fd, linectr, node)
        }
        while (node != shead);
    }
    /*if we created new file than close it */
    if(fd != NULL)
        fclose(fd);
}

/*creates '.ent' and '.ext' files */
void make_ent_file(snode *head, char *entfilename)
{
    FILE *entfd = NULL;
    /*if the list isn't empty make the '.ent' file*/
    if(head != NULL)
    {
        snode *node = head;
        /*write each line until list is completed*/
        do
        {
            if(node -> entry == TRUE)
            {
                /*if its the first enty make new file*/
                if(entfd == NULL)
                    entfd = fopen(entfilename, "w");
                fprintf(entfd, "%s  %d\n", node -> name, node -> value);
            }
            node = node -> next;
        }
        while (node != head);
    }
    /*close opened files */
    if(entfd != NULL)
        fclose(entfd);
}

/*creates '.ext' and '.ext' files */
void make_ext_file(snode *head, char *entfilename)
{
    FILE *extfd = NULL;
    /*if the list isn't empty make the '.ext' file*/
    if(head != NULL)
    {
        snode *node = head;
        /*create new file*/
        extfd = fopen(entfilename, "w"); 
        /*write each line until list is completed*/  
        do
        {
            fprintf(extfd, "%s  %d\n", node -> name, node -> value);
            node = node -> next;
        }
        while (node != head);
    }
    /*close opened files */
    if(extfd != NULL)
        fclose(extfd);
}

/*converts an int to character array which represents our PC special base 4 system*/
void to_special_quaternary(unsigned data, FILE *fd)
{
    int i;
    int mask = 3;   /*we will check the last 2 digits each time so we want binary number 11 which is 3*/
    /*run down from the last array cell and place he character we want*/
    for(i = 6; i >= 0; i--)
    {
        if(((data >> (i * 2)) & mask) == 0)
            fputc(SP_0, fd);
        else if(((data >> (i * 2)) & mask) == 1)
            fputc(SP_1, fd);
        else if(((data >> (i * 2)) & mask) == 2)
            fputc(SP_2, fd);
        else
            fputc(SP_3, fd);
    }
    fputc('\n', fd);
}