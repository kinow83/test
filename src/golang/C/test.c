
##include <stdlib.h>
typedef struct _PERSON {
    char *name;
    int age;
} PERSON;


PERSON* create(char *name, int age) {
    PERSON *p = (PERSON*)malloc(sizeof(PERSON));

    p->name = name;
    p->age = age;

    return p
}