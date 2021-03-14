/*this program is an assembler for an imaginary computer and a given assembly language.
    Author: Arie Gruber*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

int main(int argc, char *argv[])
{
    FILE *fd;
    char path[MAX_PATH_LENGTH];
    /*checks if the user entered at least one file*/
    if(argc < 2)
        NO_FILES_ERROR;

    while(--argc > 0)
    {
        int makefiles = TRUE;
        int inslines;
        int datlines;
        snode *symbhead = NULL;    /*head of symbol table list*/
        dnode *dathead = NULL;     /*haed of data lines list*/
        dnode *inshead = NULL;     /*head of instructions list*/
        snode *extlist = NULL;      /*head of ext file*/
        /*creates string with path name with '.as' extension */
        strcpy(path, argv[argc]); 
        strcat(path, ".as");
        /*checks if a file with the given name is exists, if so it opens it.
        otherwise an error will be displays and the program will terminate
        will give error also if the user is unauthorized to open the file*/
        if((fd = fopen(path, "r")) == NULL)
            NO_FILENAME_ERROR(path);
        /*the first_phase function goes through the assembly code, builds symbol table,
         computes the number of words needed for the data and code in the output files
         and checks for some errors.
         if the function finds an error it will return FALSE, and we don't need to go to phase two with this file*/
        if((makefiles = first_phase(fd, path, &symbhead, &dathead, &inslines, &datlines)))
            makefiles = second_phase(fd, path, &symbhead, &inshead, &extlist);
        /*if the code is good, make the files. when we make the cobject file we will check if we need entry and extern files as well*/
        make_files(extlist, symbhead, dathead, inshead, argv[argc], inslines, datlines);
        /*release allocactetd memory */
        clear_sym_list(symbhead, symbhead);
        clear_sym_list(extlist, extlist);
        clear_dat_list(dathead, dathead);
        clear_dat_list(inshead, inshead);
    }
    /*close opened file */
    fclose(fd);

    return 0;
}