#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "errors.h"
#include "analyzeline.h"
#include "bool.h"
#include "dsnodes.h"

int namelegit(char *word);
/*the first_phase function goes through the assembly code, builds symbol table,
computes the number of words needed for the data and code in the output files
and checks for some errors.
returns if any errors where found or not*/
int codeinstructions(snode **, dnode **head, snode **symbhead, int numofwords, enum commands com, char *origop, char *destop,
                     enum address_type origadd, enum address_type destadd, int linenumber, char *path, int *);
int first_phase(FILE *fd, char *path, snode **symbhead, dnode **dathead, int *inslines, int *datlines)
{
    int ic = 0; /*instructions counter */
    int dc = 0; /*data counter */
    char line[LINE_MAX + 2];    /*ilne container*/
    int linenumber = 0; /*counts the lines for the error messages*/
    int codeisok = TRUE;    /*if an error will be detected it will be set to FALSE*/
    while(!feof(fd))
    {
        char label[MAX_LABLE_NAME_LEN]; /*will contain the label if there is one*/
        char *word; /*word pointer*/
        int labledstatement = FALSE;    /*will be TRUE if the statement have a legit label*/
        int linelength; /*length of line*/

        enum instructions ins = non_instruction;  /*will store the type of instruction (if the statement is an instruction) */
        enum commands com = non_command;  /*will store the type of command (if the statement is a command) */
        enum statement_type sttype = not_valid; /*will store the statement type (and if the statement is valid or not) */

        /*update line number*/
        linenumber++;
        /*read next line*/
        linelength = read_line(fd, line);
        /*if line length is more than allowed maximum length print an error and go to next line */
        if(linelength > LINE_MAX)
            ERROR_MESSAGE(codeisok, linenumber, path, "line is too long. line may contain up to 80 characters")
        else
        {
            /*puts first word in the character array 'word' */
            word = strtok(line, " \t");
            /*analyze first word*/
            analyze_statement_type(word, &ins, &com, &sttype, &labledstatement, linenumber, path);
            /*if the statement is empty or a comment statement go to next line*/
            if(sttype != empty && sttype != comment)
            {
                /*if first word is a start of a macro statement*/
                if(sttype == macro)
                {
                    char *macroname;    /*pointer to macro name*/
                    char *restofline;   /*pointer to the rest of the line (without leading whitespaces)*/
                    int macrovalue; /*will contain the macro value in decimal*/
                    restofline = strtok(NULL, "");
                    /*checks for the structure of the statement */
                    if(!macro_def_ok(restofline))
                        MACRO_ERROR(linenumber, path)
                    else
                    {
                        /*sets macroname pointer to point on the macro name*/
                        macroname = strtok(restofline, " \t=");
                        /*now restofline will store the macro value(!)*/
                        restofline = strtok(NULL, " \t=");
                        /*checks if name OK, number OK and if there isnwt any non-whitespace characters after number */
                        if(!namelegit(macroname) || (macro_is_in_symtable(*symbhead, macroname, FALSE) != NULL) || !number_is_legit(restofline) || strtok(NULL, "") != NULL)
                            MACRO_ERROR(linenumber, path)
                        else
                        /*statement is OK! */
                        {
                            /*converts macro value to integer*/
                            macrovalue = atoi(restofline);
                            /*add macro to symbol table*/
                            add_sym_node(symbhead, macroname, macrovalue, symtype_mcro, 0);
                        }
                        
                    }
                }
                else
                {
                    /*in case the first word is a label save this word and read the next one*/
                    if(labledstatement)
                    {
                        int labellen = strlen(word);    /*label length*/
                        /*puts label in the 'label' variable*/
                        strcpy(label, word); 
                        /*checks if the label ends with ':', if it does remove it from the word, else replace the first letter so that the label will be wrong */
                        if(label[labellen - 1] == ':')
                        {
                            label[labellen - 1] = '\0';
                        }
                        else
                            label[0] = '0';
                        /*checks if label name is OK */
                        if(!namelegit(label) || (macro_is_in_symtable(*symbhead, label, FALSE) != NULL))
                        {
                            MACRO_ERROR(linenumber, path)
                            labledstatement = FALSE;
                        }
                        /*word will point to the next non-space character in the statement*/
                        word = strtok(NULL, " \t");
                        /*analyze first word after the label*/
                        analyze_statement_type(word, &ins, &com, &sttype, &labledstatement, linenumber, path);
                    }
                    /*if statement is labled and is a macro statement thats an error */
                    if(labledstatement && sttype == macro)
                        ERROR_MESSAGE(codeisok, linenumber, path, "macro statement started with label or illegal macro name.")
                    /*in case the statement is data or string instruction */
                    else if(ins == data || ins == string)
                    {
                        char *restofline = strtok(NULL, "");       /*make a rest of line string */
                        int restoflinelen = strlen(restofline); /*length of the rest of the line*/
                        int numofwords = 0; /*counter for words to code in data list*/
                        /*if the statement is a string instruction*/
                        if(ins == string)
                        {
                            /*checks if the word is inside a parentheses or if there are non-space characters after the the string*/
                            if(restofline[0] != '\"' || restofline[restoflinelen - 1] != '\"' || restoflinelen < 1) /*note : if numofwords = 1 than the string is legal and empty */
                                ERROR_MESSAGE(codeisok, linenumber, path, "illegal instruction statement.")
                            else
                            {                      
                                /*insert characters to the data list */
                                while(++numofwords < restoflinelen - 1)
                                    add_dat_node(dathead, restofline[numofwords]);
                                /*end word with '\0'*/
                                add_dat_node(dathead, 0);
                            }
                        }
                        /*in case this is a data statement */
                        else   
                        {
                            int numvalue;
                            /*checks if data starts or ends with comma, or if there are two commas in a row */
                            if(restofline[0] == ',' || twoconscommas(restofline) || restofline[restoflinelen - 1] == ',')
                                ERROR_MESSAGE(codeisok, linenumber, path, "illegal instruction statement.")
                            else
                            {
                                /*put first parameter in 'word' if it exist */
                                word = strtok(restofline, " \t,");
                                /*while there are more parameters to read */
                                while(word != NULL)
                                {
                                    /*checks if the parameter is a valid number. if not it can be a macro*/
                                    if(!number_is_legit(word))
                                    {
                                        snode *macnode; /*pointer to macro in symbol table*/
                                        /*macro must be declared before first use*/
                                        if((macnode =macro_is_in_symtable(*symbhead, word, TRUE)) == NULL)
                                            ERROR_MESSAGE(codeisok, linenumber, path, "data parameter must be an integer or a valid macro.")
                                        else
                                        /*add data to data list*/
                                        {
                                            numvalue = macnode -> value;
                                            add_dat_node(dathead, numvalue);
                                            /*update number of words*/
                                            numofwords++;
                                        }
                                    }
                                    else
                                    /*add data to data list*/
                                    {
                                        numvalue = atoi(word);
                                        add_dat_node(dathead, numvalue);
                                        /*update number of words*/
                                        numofwords++;
                                    }
                                    /*read next parameter */
                                    word = strtok(NULL, " \t,");
                                }
                            }
                        }
                        /*if the statement have a label instert this instrucrtion to symbol table*/
                        if(labledstatement)
                            add_sym_node(symbhead, label, dc, symtype_data, numofwords);
                        /*update dc*/
                        dc += numofwords;
                    }
                    /*in case the statement is an entry or extern instruction */
                    else if(ins == entry || ins == extrn)
                    {
                        word = strtok(NULL, " \t"); /*word will contain the label or macroname*/
                        /*check if there are non-whitespace characters after the label or macro name*/
                        if(strtok(NULL, " \t") != NULL)
                            ERROR_MESSAGE(codeisok, linenumber, path, "only one parameter is needed in extern or entry statement.")
                        else
                        {
                            /*if the instruction is '.extern' add it to symbol table marked as external*/
                            if(ins == extrn)
                                add_sym_node(symbhead, word, 0, symtype_extern, 0);
                        }                        
                    }
                    /*in case the statement is not an instruction or macro*/
                    else
                    {
                        /*if the statement have a label save it in the symbol table*/
                        if(labledstatement)
                            add_sym_node(symbhead, label, ic + 100, symtype_code, 0);
                        /*if we get here, the statement must be a command statement. if it's not or the command isn't defined that's an error*/
                        if(sttype != command || com == non_command)
                            ERROR_MESSAGE(codeisok, linenumber, path, "expected valid command name.")
                        /*check if the statement is a valid command statement*/
                        else
                        {
                            char *restofline = strtok(NULL, "");    /*point to the next non-whitespace character after command*/
                            char *origoper = NULL; /*will store the origin operand if exist*/
                            char *destoper = NULL; /*will store the destination operand if exist*/
                            enum address_type origadd = empty_address;   /*will store the origin operand address type if exist*/
                            enum address_type destadd = empty_address;  /*will store the destination operand address type if exist*/
                            /*check if the number of operands is OK and their type */
                            switch(com)
                            {
                                case mov: case cmp: case add: case sub: case lea:
                                    /*check if the structure isn't OK*/
                                    if(!twooperands(restofline, &origoper, &destoper))
                                        OPERANDS_NOT_COMP(linenumber, path)
                                    else
                                    {
                                        origadd = get_address_type(origoper);   /*if all good will contain the address type of origin operand*/
                                        destadd = get_address_type(destoper);   /*if all good will contain the address type of destination operand*/
                                        /*checks if the operands address type match the command*/
                                        if((origadd == empty_address || destadd == empty_address) || /*in case not two valid operands*/
                                        (com == lea && origadd != direct && origadd != indx) || /*in case of non-valid origin lea operand */
                                        (com != cmp && destadd == imdt))  /*in case command is not cmp and destination operand is immediate */
                                            OPERANDS_NOT_COMP(linenumber, path)
                                        else
                                            /*update ic */
                                            ic += countwords(origadd, destadd);
                                    }
                                    break;
                                case not: case clr: case inc: case dec: case jmp: case bne: case red: case prn: case jsr:
                                    /*in case there is no operand*/
                                    if(restofline == NULL)
                                        OPERANDS_NOT_COMP(linenumber, path)
                                    else
                                    {
                                        origoper = NULL;    /*those commands need only destination operand*/
                                        destoper = strtok(restofline, " \t");
                                        /*check if there are non-whitespace characters after he operand*/
                                        if(strtok(NULL, " \t") != NULL)
                                            ERROR_MESSAGE(codeisok, linenumber, path, "only one parameter is needed for this command.")
                                        else
                                        {
                                            destadd = get_address_type(destoper); /*if the operand is not valid it will get empty_address */
                                            /*checks if the operand match the command*/
                                            if((destadd == empty_address) || /*operand is not valid */
                                            (com != prn && destadd == imdt) || /*only prn command can have an immediate destination operand */
                                            ((com == jsr || com == bne || com == jmp) && destadd == indx))
                                                OPERANDS_NOT_COMP(linenumber, path)
                                            else
                                            /*update ic */
                                            ic += countwords(empty_address, destadd);
                                        }
                                    }
                                    break;
                                case rts: case stop:
                                    /*check if any non-whitespace characters after command*/
                                    if(restofline != NULL)
                                        OPERANDS_NOT_COMP(linenumber, path)
                                    else
                                        /*update ic */
                                        ic++;
                                    break;
                                /*in case of unknown error */
                                default:
                                    ERROR_MESSAGE(codeisok, linenumber, path, "unknown error occured.")
                                    break;
                            }
                        }
                    }
                }
            }
        }
    }
    /*update all data and string values in the symbol table to its value + ic + 100*/
    update_sym_table_dat(*symbhead, *symbhead, ic + 100);
    /*updates the dc and ic variables in the main function */
    *inslines = ic;
    *datlines = dc;
    /*if the code is without any errors returns TRUE, else returns FALSE*/
    return codeisok;
}
/*the second phase builds the instruction list in 'inshead', which will be the list we will use to build the '.ob' code file*/
int second_phase(FILE *fd, char *path, snode **symbhead, dnode **inshead, snode **extlist)
{
    int codeisok = TRUE;    /*indicates if the code is OK*/
    int ic = 100;   /*counter for commands, code start at 100*/
    int linenumber = 0; /*counts the lines for the error messages*/
    char line[LINE_MAX + 2];    /*ilne container*/
    /*rewinds fd pointer to the the beggining of the file*/
    rewind(fd);
    while(!feof(fd))
    {
        char *word; /*pointer for words*/
        int labledstatement = FALSE;    /*indicates if the statement is labeled or not*/

        enum instructions ins = non_instruction;  /*will store the type of instruction (if the statement is an instruction) */
        enum commands com = non_command;  /*will store the type of command (if the statement is a command) */
        enum statement_type sttype = not_valid; /*will store the statement type (and if the statement is valid or not) */
        /*read each line*/
        read_line(fd, line);
        /*update line number counter*/  
        linenumber++;
        /*puts first word in the character array 'word' */
        word = strtok(line, " \t");
        /*analyze first word*/
        analyze_statement_type(word, &ins, &com, &sttype, &labledstatement, linenumber, path);
        /*if the statement is empty or a comment or a macro statement go to next line*/
        if(sttype != empty && sttype != comment && sttype != macro)
        {
            /*ignore  label if exist*/
            if(labledstatement)
            {
                word = strtok(NULL, " \t"); /*word will point to the next non-whitespace character*/
                /*analyze the type of the statement*/
                analyze_statement_type(word, &ins, &com, &sttype, &labledstatement, linenumber, path);
            }
            /*if the statement is data or string  or exter go to next line*/
            if(ins != data && ins != string && ins != extrn)
            {
                /*in case of an entry statement*/
                if(ins == entry)
                {
                    snode *entname; /*will point to the node with the given name*/
                    /*make word point to the name of the macro or label*/
                    word = strtok(NULL, " \t");
                    /*check if macro or label is in symbol list*/
                    if((entname = macro_is_in_symtable(*symbhead, word, FALSE)) == NULL)
                        ERROR_MESSAGE(codeisok, linenumber, path, "entry instruction must contain a valid label or macro name.")
                    else
                        /*sets entry flag in symbol table to TRUE*/
                        entname -> entry = TRUE;
                }
                /*statement should be a code statement*/
                else
                {
                    char *restofline = strtok(NULL, "");    /*sets rest of line to point to the rest of the line*/
                    int icupdate;   /*how much we need to add to IC*/
                    char *origoper = NULL;  /*point to origin operand*/
                    char *destoper = NULL;  /*point to origin operand*/
                    enum address_type origadd = empty_address;  /*address type of origin operand*/
                    enum address_type destadd = empty_address;    /*address type of destination operand*/

                    switch(com)
                    {
                        case mov: case cmp: case add: case sub: case lea:
                            /*make the pointers to point on their operands*/
                            twooperands(restofline, &origoper, &destoper);
                            /*check the address type of the operands*/
                            origadd = get_address_type(origoper);
                            destadd = get_address_type(destoper);
                            /*calculate how many words to add to IC*/
                            icupdate = countwords(origadd, destadd);
                            break;

                        case not: case clr: case inc: case dec: case jmp: case bne: case red: case prn: case jsr:
                            /*make the pointer to point on their operands*/
                            destoper = strtok(restofline, " \t");   
                            /*check the address type of the operands*/
                            destadd = get_address_type(destoper);
                            /*calculate how many words to add to IC*/
                            icupdate = countwords(origadd, destadd);
                            break;

                        case rts: case stop:
                            icupdate = 1;
                            break;
                        /*in case of unknown error */
                        default:
                            ERROR_MESSAGE(codeisok, linenumber, path, "unknown error occured.")
                            break;
                    }             
                    /*code all of the instruction words. if error will be detected 'codeisok' variable will be set to FALSE. function also updates IC*/
                    codeisok = (codeinstructions(extlist, inshead, symbhead, icupdate, com, origoper, destoper, origadd, destadd, linenumber, path, &ic) && codeisok);
                    /* */
                }
            }
        }
    }
    /*if all went well return TRUE.*/
    return codeisok;
}