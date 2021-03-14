#define LINE_MAX 81 /*maximum line length allowed is 80 charecters, +1 for end of line signal */

/*maximum characters length of a path */
#define MAX_PATH_LENGTH 2048

/*no file was provided by the user */
#define NO_FILES_ERROR \
{ \
    printf("Program exit with error: you must provide at least one file to the assembler.\n"); \
    exit(1);\
}

/*the user entered  wrong filename / is unauthorized to open it*/
#define NO_FILENAME_ERROR(path) \
{ \
    printf("Program exit with error: no such file \"%s\", or there was a problem opening it.\n", path); \
    exit(1); \
}

/*there is an error in  a macro*/
#define MACRO_ERROR(linenumber, filename) \
{ \
    codeisok = FALSE; \
    printf("Error in line %d in file \"%s\": illegal label or macro name.\n", linenumber, filename); \
}

/*operands not compatible with command type*/
#define OPERANDS_NOT_COMP(linenumber, filename) \
{ \
    codeisok = FALSE; \
    printf("Error in line %d in file \"%s\": operands not compatible with command.\n", linenumber, filename); \
}
/*template for error messages. given 'message' chould contain the error message we want to print to the user*/
#define ERROR_MESSAGE(codeisok, linenumber, filename, message) \
{ \
    codeisok = FALSE; \
    printf("Error in line %d in file \"%s\": "message"\n", linenumber, filename); \
}

