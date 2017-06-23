#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "person.h"

typedef struct node{
    Person *person;
    struct node *prev;
    struct node *next;
} Node;


typedef struct book {
    Node *first;
    Node *prev;
} Book;


Book* create_book();

void delete_book(Book *book);

void add_contact(Person *p, Book *book);

void delete_contact(Person *p, Book *book);

Person *find_contact(Person* p, Book *book);

Book *ssort(Book *book, int i);

void print_book(Book* book);

#endif
