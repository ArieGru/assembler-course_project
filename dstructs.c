#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dsnodes.h"
#include "bool.h"

/*adds a node to snode structure*/
void add_sym_node(snode **head, char *name, int value, enum symboltypes type, int numofwords)
{
    snode *newnode = (snode *)malloc(sizeof(snode));    /*allocate memory for the new node*/
    strcpy(newnode -> name, name);  /*copy name to new node's name field*/
    newnode -> value = value;   /*copy value to new node's value field*/
    newnode -> type = type;     /*copy type to new node's type field*/
    newnode -> entry = FALSE;   /*when a new node is created we don't know yet if there is an entry decleration*/
    newnode -> numofwords = numofwords; /*if the node is an array head store the number of cells in the array*/
    /*if the list is empty point head to the new node*/
    if(*head == NULL)
    {
        newnode -> next = newnode;
        newnode -> prev = newnode;
        (*head) = newnode;
    }
    else
    /*place the new node at the end of the list*/
    {
        newnode -> next = (*head);
        newnode -> prev = (*head) -> prev;
        (*head) -> prev = newnode;
        newnode -> prev -> next = newnode;
    }
}

/*checks if the name is in the symbol table. returns a pointer(!) to the macro or NULL if it isn't there.
 lookformacro variable indicates if we only want a macro or not*/
snode *macro_is_in_symtable(snode *head, char *name, int lookformacro)
{
    /*if the list is empty return NULL*/
    if(head == NULL)
        return NULL;
    else
    {
        snode *node = head;
        do
        {
            /*check if the name is the name we are looking for*/
            if(strcmp(node -> name, name) == 0)
            {  
                /*if it is a macro or if we dont look only for a macro return the node*/
                if(node -> type == symtype_mcro || !lookformacro)
                    return node;
            }
            node = node -> next;
        }
        while (node != head);  
    }
    /*if we got here we didn't find what we looked for*/
    return NULL;
}

/*release all allocated memory in an snode list structure*/
void clear_sym_list(snode *node, snode *head)
{
    /*if the list isn't empty*/
    if(head != NULL)
    {
        if(node -> next != head)
            clear_sym_list(node -> next, head);
        free(node);
    }
}

/*updates the symbol table 'data' nodes in the end of the first phase*/
void update_sym_table_dat(snode *node, snode *head, int ic)
{
    /*if the list isn't empty*/
    if(head != NULL)
    {
        if(node -> next != head)
            update_sym_table_dat(node -> next, head, ic);
        if(node -> type == symtype_data)
            node -> value += ic;
    }
}

/*adds a node to dnode structure*/
void add_dat_node(dnode **head, int value)
{
    dnode *newnode = (dnode *)malloc(sizeof(dnode));    /*allocate memory for the new node*/
    newnode -> value = value;   /*copy value to new node's value field*/
    /*if the list is empty point head to the new node*/
    if(*head == NULL)
    {
        newnode -> next = newnode;
        newnode -> prev = newnode;
        (*head) = newnode;
    }
    else
    /*place the new node at the end of the list*/
    {
        newnode -> next = *head;
        newnode -> prev = (*head) -> prev;
        (*head) -> prev = newnode;
        newnode -> prev -> next = newnode;
    }
}

/*release all allocated memory in an dnode list structure*/
void clear_dat_list(dnode *node, dnode *head)
{
    /*if the list isn't empty*/
    if(head != NULL)
    {
        if(node -> next != head)
            clear_dat_list(node -> next, head);
        free(node);
    }
}
