enum symboltypes {symtype_data, symtype_code, symtype_mcro, symtype_extern};

/*this structure will contain the symbol table. we also use it for the ext list for the executable file*/
typedef struct symbolnode 
{
    struct symbolnode *next;    /*pointer to the next node*/
    struct symbolnode *prev;    /*pointer to the previous node*/
    char name[80];  /*node's name*/
    int value;  /*node's value*/
    enum symboltypes type;  /*node's type*/
    int entry;  /*TRUE if node name have an entry decleration in the code*/
    int numofwords; /*non-zero only if symbol is an array. indicates it's length */
} snode;

/*this structure will contain the data table */
typedef struct datanode 
{
    struct datanode *next;  /*pointer to the next node*/
    struct datanode *prev;  /*pointer to the previous node*/
    int value;    /*node's value*/
    int external;   /*indicates if the node is external or not*/
} dnode;

/*adds a node to snode structure*/
void add_sym_node(snode **head, char *name, int value, enum symboltypes type, int numofword );
/*checks if the name is in the symbol table. returns a pointer(!) to the macro or NULL if it isn't there.
 lookformacro variable indicates if we only want a macro or not*/
snode *macro_is_in_symtable(snode *head, char *name, int);
/*release all allocated memory for in an snode list structure*/
void clear_sym_list(snode *node, snode *head);
/*updates the symbol table 'data' nodes in the end of the first phase*/
void update_sym_table_dat(snode *node, snode *head, int ic);
/*adds a node to dnode structure*/
void add_dat_node(dnode **head, int value);
/*release all allocated memory in an dnode list structure*/
void clear_dat_list(dnode *node, dnode *head);
