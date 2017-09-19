package main

/*
#include <stdlib.h>
#include <stdio.h>

int sum(int a, int b) {
	return a + b;
}

void hello() {
	printf("Hellow, World! C\n");
}
 */
import "C"
import "fmt"

func main() {
	r := C.rand()
	fmt.Println(r)

	var a, b int = 10, 20
	r = C.sum(C.int(a), C.int(b))
	fmt.Println(r)

	r = C.sum(C.int(20), C.int(30))
	fmt.Println(r)

	C.hello()
}
