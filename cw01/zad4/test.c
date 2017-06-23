//zad4
#include <sys/resource.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "person.h"
#include "list.h"
#include "BST.h"

struct timeval ts_before, tu_before, tr_before, ts, tu, tr;


void print_times(){

    long int utime = (tu.tv_sec-tu_before.tv_sec)*1e6 + tu.tv_usec-tu_before.tv_usec;
    long int stime = (ts.tv_sec-ts_before.tv_sec)*1e6 + ts.tv_usec-ts_before.tv_usec;
    long int rtime = (tr.tv_sec-tr_before.tv_sec)*1e6 + tr.tv_usec-tr_before.tv_usec;
    utime /= 1e3;
    stime /= 1e3;
    rtime /= 1e3;
    printf("Czas rzeczywisty: %ld ms\n", rtime);
    printf("Czas uzytkownika: %ld ms\n", utime);
    printf("Czas systemu: %ld ms\n", stime);
}

void get_time_before(struct rusage *usage)
{
    gettimeofday(&tr_before, NULL);
    getrusage(RUSAGE_SELF, usage);
    tu_before=usage->ru_utime;
    ts_before=usage->ru_stime;
}

void get_time(struct rusage *usage)
{
    gettimeofday(&tr, NULL);
    getrusage(RUSAGE_SELF, usage);
    tu=usage->ru_utime;
    ts=usage->ru_stime;
    print_times();
}

void read(FILE *fstream, Book *book, struct rusage *usage) {

    char buffer[1024], **atribute;
    char *line, *record;
    int i, j, k;
    Person *p;

    atribute=malloc(6*sizeof(char*));
    k=0;
    line=fgets(buffer, sizeof(buffer), fstream);
    while(line!=NULL)
    {
        i=0;
        record = strtok(line,",");
        while(record != NULL)
        {
            atribute[i]=malloc(strlen(record)*sizeof(char));
            for(j=0; j<strlen(record); j++)
            {
                atribute[i][j]=record[j];
            }
            record = strtok(NULL,",");
            i++;

        }
        p=create_person(atribute[0], atribute[1], atribute[2], atribute[3], atribute[4], atribute[5]);
        if(k==0) {
            if(book) printf("----Lista---- \nDodawanie kontaktu:\n");
            else printf("----BST---- \nDodawanie kontaktu:\n");
            get_time_before(usage);
        }

        if(book) add_contact(p, book);
        else add_contact_tree(p, root);

        if(k==0) {get_time(usage); k++;}
        line=fgets(buffer, sizeof(buffer), fstream);
    }
    free(atribute);
}

int main(void) {

    struct rusage *usage=malloc(sizeof(*usage));

    Book *book = create_book();

    FILE *fstream = fopen("../zad3a/MOCK_DATA.csv","r");

    if(fstream == NULL) {
        printf("Cannot open file\n");
        return -1;
    }

    read(fstream, book, usage);
    printf("Tworzenie:\n");
    get_time(usage);

    Person *p=create_person("Amy","Little","1999/07/09","alittlerq@gmpg.org","888-45-3321","5 Sherman Street");
    printf("Szukanie optymistyczne:\n");
    get_time_before(usage);
    find_contact(p, book);
    get_time(usage);

    printf("Usuwanie optymistyczne:\n");
    get_time_before(usage);
    delete_contact(p, book);
    get_time(usage);
    free(p);

    //p=create_person("Charles","James","1983/08/01","cjames0@marriott.com","547-68-1965","33367 Delaware Plaza");
    printf("Szukanie pesymistyczne:\n");
    get_time_before(usage);
    find_contact(p, book);
    get_time(usage);

    printf("Usuwanie pesymistyczne:\n");
    get_time_before(usage);
    delete_contact(p, book);
    get_time(usage);
    free(p);

    printf("Sortowanie:\n");
    get_time_before(usage);
    book=ssort(book, 4);
    get_time(usage);

    delete_book(book);

    rewind(fstream);

    create_booktree();

    read(fstream, NULL, usage);
    printf("Tworzenie:\n");
    get_time(usage);

    p=create_person("Charles","James","1983/08/01","cjames0@marriott.com","547-68-1965","33367 Delaware Plaza");
    printf("Szukanie optymistyczne:\n");
    get_time_before(usage);
    find_contact_tree(root, p);
    get_time(usage);

    printf("Usuwanie optymistyczne:\n");
    get_time_before(usage);
    delete_contact_tree(p);
    get_time(usage);
    free(p);

    p=create_person("Amy","Little","1999/07/09","alittlerq@gmpg.org","888-45-3321","5 Sherman Street");
    printf("Szukanie pesymistyczne:\n");
    get_time_before(usage);
    find_contact_tree(root, p);
    get_time(usage);

    printf("Usuwanie pesymistyczne:\n");
    get_time_before(usage);
    delete_contact_tree(p);
    get_time(usage);
    free(p);

    printf("Przebudowa:\n");
    get_time_before(usage);
    rebuild_tree(root, 1);
    get_time(usage);

    delete_booktree(root);

    fclose(fstream);
    free(usage);

    return 0;



}
