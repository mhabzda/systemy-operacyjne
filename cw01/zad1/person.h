#ifndef PERSON_H
#define PERSON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct person {
    char* name;
    char* last_name;
    char* birth_date;
    char* email;
    char* phone;
    char* address;
} Person;

Person *create_person(char* name, char* last_name, char* birth_date, char* email, char* phone, char* address);

void destroy_person(Person *p);

int person_equals(Person p1, Person p2);

void print_person(Person p);

#endif
