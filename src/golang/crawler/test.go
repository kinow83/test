package main
import (
	"sync"
	"fmt"
)


var fetched = struct{
	m map[string] error
	sync.Mutex
} {m: make(map[string]error)}

type E struct {
}
func (e E) Error() string {
	return "e"
}

func main() {

	e := E{}

	fetched.m["aa"] = e

	if a, ok := fetched.m["aa"]; ok {
		fmt.Println(ok, a)
	} else {
		fmt.Println(ok, a)
	}
}