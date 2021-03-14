#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "bool.h"
#include "errors.h"
#include "analyzeline.h"

/*list of reserved words*/
#define RES_WORDS_NUM 29
char reswords[RES_WORDS_NUM][10] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "define", "data", "string", "entry", "extern", 
                        "mov", "cmp", "add", "sub", "not", "clr", "lea", "inc", "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop"};

/*checks if the label name is OK */
int namelegit(char *word)
{
    int i = 0;
    /*first letter must be an alphabetic letter */
    if(!isalpha(word[0]))
        return FALSE;
    /*checks if all letters are legit and the length of the label */
    while(word[i] != '\0')
    {
        if(!isalpha(word[i]) && !isdigit(word[i]))
            return FALSE;
        i++;
    }
    /*name must be less than 'MAX_LABLE_NAME_LEN' characters*/
    if(i >= MAX_LABLE_NAME_LEN)
        return FALSE;
    /*checks if word is a reserved word*/
    for(i = 0; i < RES_WORDS_NUM; i++)
    {
        if(strcmp(word, reswords[i]) == 0)
            return FALSE;
    }
    return TRUE;
}
/*checks if a word is a command word, and returns wich one it is */
enum commands analyze_command(char *word)
{
    enum commands com;
    if(strcmp(word, "mov") == 0)
        com = mov;
    else if(strcmp(word, "cmp") == 0)
        com = cmp;
    else if(strcmp(word, "add") == 0)
        com = add;
    else if(strcmp(word, "sub") == 0)
        com = sub;
    else if(strcmp(word, "not") == 0)
        com = not;
    else if(strcmp(word, "clr") == 0)
        com = clr;
    else if(strcmp(word, "lea") == 0)
        com = lea;
    else if(strcmp(word, "inc") == 0)
        com = inc;
    else if(strcmp(word, "dec") == 0)
        com = dec;
    else if(strcmp(word, "jmp") == 0)
        com = jmp;
    else if(strcmp(word, "bne") == 0)
        com = bne;
    else if(strcmp(word, "red") == 0)
        com = red;
    else if(strcmp(word, "prn") == 0)
        com = prn;
    else if(strcmp(word, "jsr") == 0)
        com = jsr;
    else if(strcmp(word, "rts") == 0)
        com = rts;
    else if(strcmp(word, "stop") == 0)
        com = stop;
    else
        com = non_command;
    return com;

}

/*checks if a word is an instriction word, and returns wich one it is */
enum instructions analyze_instructions(char *word)
{
    enum instructions ins;
    if(strcmp(word, ".data") == 0)
        ins = data;
    else if(strcmp(word, ".string") == 0)
        ins = string;
    else if(strcmp(word, ".entry") == 0)
        ins = entry;
    else if(strcmp(word, ".extern") == 0)
        ins = extrn;
    else
        ins = non_instruction;
    return ins;
    
}

/*this function will read a line to the character array 'line'*/
int read_line(FILE *fd, char *line)
{
    int c; /*will contain each character in the line */
    int ctr = 0; /*counts the number of characters in the line */
    /*ignore the whitespaces at the start of each line*/
    while((c = fgetc(fd)) != EOF && c != '\n' && isspace(c))
    ;
    /*read each character until end of file or new line is detected*/
    while(c != EOF && c != '\n')
    {
        /*while the line is less than maximum line length, put the character in the line c ontainer*/
        if(ctr < LINE_MAX)
            line[ctr] = c;
        ctr++;
        c = fgetc(fd);
    }
    
    /*puts end of line signal*/
    if(ctr < LINE_MAX)
        line[ctr] = '\0';

    /*returns length of line*/
    return ctr;
    
}

/*this function analyzes the first word (or second if there is a label) */
void analyze_statement_type(char *word, enum instructions *ins, enum commands *com, enum statement_type *sttype, int *labledstatement, int linenumber, char *path)
{
    /*empty statement*/
    if(word == NULL)
        *sttype = empty;
    /*in case the statement is a comment*/
    else if(word[0] == ';')
        *sttype = comment;
    /*in case the word is an instruction word */
    else if((*ins = analyze_instructions(word)))
        *sttype = instruction;
    /*in case the word is a command word */
    else if((*com = analyze_command(word)))
        *sttype = command;
    /*in case of a macro definition */
    else if(strcmp(word, ".define") == 0)
        *sttype = macro;
    /*if none of the above it should be a label*/
    else
        *labledstatement = TRUE;
}

/*checks if there are two commas in a row (between white spaces or not),
and if the first and last non-whitespace character isn't a comma*/
int twoconscommas(char *line)
{
    int i = 0;
    int comma = FALSE;
    int linelen = strlen(line);
    char *lastchar= (line + linelen); /*points to the last character in line*/
    /*make 'lastchar' point to the last non-whitespace character*/
    while(lastchar != line && isspace(*(--lastchar)))
    ;
    /*if line point to NULL return FALSE*/
    if(line == NULL)
        return FALSE;
    /*check if first or last characters is a comma*/
    if(line[0] == ',' || *lastchar == ',')
        return TRUE;
    while(line[i] != '\0')
    {
        if(line[i] == ',' && comma == TRUE)
            return TRUE;
        else if(line[i] == ',')
            comma = TRUE;
        else if(!isspace(line[i]))
            comma = FALSE;
        i++;
    }
    return FALSE;
}

/*checks if the word is a valid integer */
int number_is_legit(char *word)
{
    int i = 0;
    int digits = FALSE;
    /*number must start with '+', '-', or a digit*/
    if(!isdigit(word[0]) && word[0] != '+' && word[0] != '-')
        return FALSE;
    /*if the first character is a digit set digits to TRUE*/
    if(isdigit(word[0]))
        digits = TRUE;
    while(word[++i] != '\0')
    {
        /*checks if a character is a digit or not*/
        if(!isdigit(word[i]))
            return FALSE;
        else
            digits = TRUE;
    }
    /*if there is only a plus or minus sign it will return FALSE*/
    return digits;
}

/*checks if only one '=' sign in the statement and if it's not in the start or end. also return false if line is empty */
int macro_def_ok(char *line)
{
    int howmany = 0;
    int linelen = strlen(line);
    /*macro statement cant start or end with '='*/
    if(line[0] == '=' || line[linelen - 1] == '=' || line[0] == '\0')
        return FALSE;
    else
    {
        int i = 0;
        /*count how many '=' signs in the line*/
        while(line[++i] != '\0')        
            if(line[i] == '=')
                howmany++;
    }
    return howmany == 1;
}
/*checks if there are two operands in the line without extra non-whitespace characters*/
int twooperands(char *line, char **origoper, char **destoper)
{
    int numofcommas = 0;    /*need only one comma*/
    int i = 0;  /*index counter*/
    int linelen = strlen(line);
    char *lastchar= (line + linelen); /*points to the last character in line*/
    /*make 'lastchar' point to the last non-whitespace character*/
    while(lastchar != line && isspace(*(--lastchar)))
    ;
    /*check if line start or end with a comma*/
    if(line[0] == ',' || (*lastchar) == ',')
        return FALSE;
    /*check that there is only one comma*/
    while (line[i] != '\0')
    {
        if(line[i] == ',')
            numofcommas++;
        i++;
    }
    *origoper = strtok(line, " \t,");
    *destoper = strtok(NULL, " \t,");
    /*check if there are extra non-whitespace characters*/
    if(strtok(NULL, "") != NULL)
        return FALSE;
    return TRUE;
}
/*returns the address type*/
enum address_type get_address_type(char *operand)
{
    int operlen = strlen(operand);
    /*if there is no operand*/
    if(operand == NULL)
        return empty_address;
    /*if the operand strarts with '#' it should be an immediate address type operand. we only check here if it begins with '#'*/
    if(operand[0] == '#')
        return imdt;
    /*check if the operand ends with ']' it should indexed address type operand. we only check if the structure of the brackets is OK and for spaces between them*/
    else if(operand[operlen - 1] == ']')
    {
        /*check if structure is ok*/
        int i;
        int rightone = FALSE;
        for(i = 0; i < operlen; i++)
        {
            if(operand[i] == '[' && !rightone)
                rightone = TRUE;
            /*if there are two '[' or two ']' signs in the operand it is illegal. also there shouln't be space between the brackets*/
            else if(operand[i] == '[' || (operand[i] == ']' && i != operlen - 1) || (rightone && isspace(operand[i])))
                return empty_address;
        }
        return indx;
    }
    /*checks if its a register name. no more checks is required if it is (!)*/
    else if(strcmp(operand, "r0") == 0 ||
            strcmp(operand, "r1") == 0 ||
            strcmp(operand, "r2") == 0 ||
            strcmp(operand, "r3") == 0 ||
            strcmp(operand, "r4") == 0 ||
            strcmp(operand, "r5") == 0 ||
            strcmp(operand, "r6") == 0 ||
            strcmp(operand, "r7") == 0)
        return regis;
    /*checks if the word is a valid word. if it does the operand is a direct address type operand*/
    else if(namelegit(operand))
        return direct;
    /*if none of the above the operand is not a valid operand*/
    else
        return empty_address;
}

/*count how many words we need to add*/
int countwords(enum address_type origadd, enum address_type destadd)
{
    int count = 1;
    if(origadd == regis && destadd == regis)
        return 2;
    if(origadd == indx)
    {
        count += 2;
    }
    else if(origadd == regis || origadd == direct || origadd == imdt)
        count++;
    if(destadd == indx)
        count += 2;
    else if(destadd == regis || destadd == direct || destadd == imdt)
        count++;
    return count;
}
