package main

/*
#include <stdlib.h>
#include <stdio.h>

typedef struct _PERSON {
    char *name;
    int age;
} PERSON;


PERSON* create(char *name, int age) {
    PERSON *p = (PERSON*)malloc(sizeof(PERSON));

    p->name = name;
    p->age = age;

    return p;
}

void show_person(PERSON *p) {
	printf("name: %s\n", p->name);
	printf("age: %d\n", p->age);
}
*/
import "C"
import (
	"unsafe"
	"fmt"
	"reflect"
)

func main() {
	var p *C.PERSON

	name := C.CString("kinow")
	age := C.int(33)

	p = C.create(name, age)

	C.show_person(p)

	fmt.Println("reflect PERSON: ", reflect.TypeOf(p))
	fmt.Println("reflect char* of PERSON: ", reflect.TypeOf(p.name))
	fmt.Println("reflect int of PERSON: ", reflect.TypeOf(p.age))
	fmt.Println(C.GoString(p.name))
	fmt.Println(p.age)

	C.free(unsafe.Pointer(name))
	C.free(unsafe.Pointer(p))

}