#ifndef BST_H
#define BST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "person.h"

typedef struct nodeBT {
    Person *person;
    struct nodeBT *left;
    struct nodeBT *right;
    struct nodeBT *parent;

} NodeBT;
NodeBT *root;
NodeBT *root_rebuilt;
int atribute;

void create_booktree();

void delete_booktree(NodeBT* start);

void add_contact_tree(Person *p, NodeBT *start);

NodeBT* find_contact_tree(NodeBT *start, Person *p);

void delete_contact_tree(Person *p);

void rebuild_tree(NodeBT* start, int which_atribute);

void print_tree(NodeBT *root);

#endif
