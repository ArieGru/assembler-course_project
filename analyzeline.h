#define EOF_REACHED 0
#define NOT_EOF 1

#define MAX_LABLE_NAME_LEN 32

enum statement_type {not_valid, empty, comment, instruction, command, macro};
enum instructions {non_instruction, data, string, entry, extrn};
enum commands {non_command, mov, cmp, add, sub, not, clr, lea, inc, dec, jmp, bne, red, prn, jsr, rts, stop};
enum address_type {imdt, direct, indx, regis, empty_address};

/*checks if the label name is OK */
int labellegit(char *);
/*checks if a word is a command word, and returns wich one it is */
enum commands analyze_command(char *);
/*checks if a word is an instriction word, and returns wich one it is */
enum instructions analyze_instructions(char *);
/*this function will read a line to the character array 'line'*/
int read_line(FILE *, char *);
/*this function analyzes the first word (or second if there is a label) */
void analyze_statement_type(char *, enum instructions *, enum commands *, enum statement_type *, int *, int, char *);
/*checks if there are two commas in a row (between white spaces or not) */
int twoconscommas(char *);
/*checks if the word is a valid integer */
int number_is_legit(char *);
/*checks if only one '=' sign in the statement and if it's not in the start or end. also return false if line is empty */
int macro_def_ok(char *);
/*checks if there are two operands in the line */
int twooperands(char *, char **, char **);
/*returns the address type*/
enum address_type get_address_type(char *);
/*count how many words we need to add*/
int countwords(enum address_type , enum address_type );
