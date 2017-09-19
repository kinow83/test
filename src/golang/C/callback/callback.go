package main
import (
	"unsafe"
	"fmt"
)


/*
extern void goCallback(void *p, int n);

static inline void CExample(void *p) {
	goCallback(p, 100);
}
 */
import "C"

//export goCallback
func goCallback(p unsafe.Pointer, n C.int) {
	f := *(*func(C.int))(p)
	f(n)
}

func main() {
	f := func(n C.int) {
		fmt.Println(n)
	}

	C.CExample(unsafe.Pointer(&f))
}
