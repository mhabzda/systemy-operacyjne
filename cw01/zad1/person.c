#include "person.h"

Person *create_person(char* name, char* last_name, char* birth_date, char* email, char* phone, char* address) {
    Person *new_person=malloc(sizeof(*new_person));
    if(new_person!=NULL) {
        new_person->name = name;
        new_person->last_name = last_name;
        new_person->birth_date = birth_date;
        new_person->email = email;
        new_person->phone = phone;
        new_person->address = address;
    }

    return new_person;
}

void destroy_person(Person *p) {
    if(p==NULL) return;

    free(p->name);
    free(p->last_name);
    free(p->birth_date);
    free(p->email);
    free(p->phone);
    free(p->address);
    free(p);
}

int person_equals(Person p1, Person p2) {
    if(strcmp(p1.name, p2.name)!=0) return -1;
    if(strcmp(p1.last_name, p2.last_name)!=0) return -1;
    if(strcmp(p1.birth_date, p2.birth_date)!=0) return -1;
    if(strcmp(p1.email, p2.email)!=0) return -1;
    if(strcmp(p1.phone, p2.phone)!=0) return -1;
    if(strcmp(p1.address, p2.address)!=0) return -1;
    return 0;
}

void print_person(Person p) {
    printf("%s %s %s %s %s %s\n", p.name, p.last_name, p.birth_date, p.email, p.phone, p.address);

}
