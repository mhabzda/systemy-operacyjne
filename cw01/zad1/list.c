#include "list.h"

Node* create_node(Person *p){
    Node *new_node = malloc(sizeof(*new_node));
    if (new_node != NULL) {
        new_node->person = p;
        new_node->next=NULL;
        new_node->prev=NULL;
    }
    return new_node;
}

Book* create_book() {
    Book *b = malloc(sizeof(*b));
    b->first = NULL;
    b->prev=NULL;
    return b;
}

void delete_book(Book *book) {
    Node *p=book->first, *q;
    if(p!=NULL) q=p->next;
    while(p!=NULL) {
        destroy_person(p->person);
        free(p);
        p=q;
        if(q!=NULL) q=q->next;
    }
    free(book);
}

void add_contact(Person *p, Book *book) {
    Node *node=create_node(p);
    if(book->first==NULL) {
        book->first=node;
        book->prev=node;
    }
    else {
        node->next=book->first;
        book->first->prev=node;
        book->first=node;
    }
}

void delete_contact(Person *p, Book *book) {
    Node *q = book->first;
    while(q!=NULL) {
        if(person_equals(*p, *(q->person))==0) {
            Node *r1=q->prev;
            Node *r2=q->next;
            if(r1!=NULL) r1->next=q->next;
            else book->first=q->next;
            if(r2!=NULL) r2->prev=q->prev;
            else book->prev=q->prev;
			destroy_person(q->person);
            free(q);
            q=NULL;
        }
        if(q!=NULL) q=q->next;
    }
}

Person *find_contact(Person *p, Book *book) {
    Node *q = book->first;
    while(q!=NULL) {
        if(person_equals(*p, *(q->person))==0) return q->person;
        if(q!=NULL) q=q->next;
    }
    return NULL;
}

Node *delmax(Book *book, int i) {
    Node *p=book->first;
    Node *q=book->first;
    while(q!=NULL) {
        if(i==1) {
            if(strcmp(p->person->last_name, q->person->last_name)<0) p=q;
        }
        else if(i==2) {
            if(strcmp(p->person->birth_date, q->person->birth_date)<0) p=q;
        }
        else if(i==3) {
            if(strcmp(p->person->email, q->person->email)<0) p=q;
        }
        else {
            if(strcmp(p->person->phone, q->person->phone)<0) p=q;
        }

        if(q!=NULL) q=q->next;
    }
    Node *r1=p->prev;
    Node *r2=p->next;
    if(r1!=NULL) r1->next=p->next;
    else book->first=p->next;
    if(r2!=NULL) r2->prev=p->prev;
    else book->prev=p->prev;
    p->next=NULL;
    p->prev=NULL;
    return p;
}

Book *ssort(Book *book, int i) {
    Book *book_sorted = create_book();
    while(book->first!=NULL) {
        Node *r=delmax(book, i);
        if(book_sorted->first==NULL) {
            book_sorted->first=r;
            book_sorted->prev=r;
        }
        else {
            r->next=book_sorted->first;
            book_sorted->first->prev=r;
            book_sorted->first=r;
        }
    }
    free(book);
    return book_sorted;

}

void print_book(Book* book) {
    Node *p=book->first;
    while(p!=NULL) {
        print_person(*(p->person));
        p=p->next;
    }
    printf("\n");
}
