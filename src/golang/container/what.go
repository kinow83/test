package main
import (
	"fmt"
	"reflect"
)


type Myint []int

func (m *Myint) Plen() int {
	return len(*m)
}

func (m Myint) Len() int {
	return len(m)
}

func main() {
	m := Myint{1, 2, 3}

	fmt.Println(m)
	fmt.Println(reflect.TypeOf(m))

	fmt.Println(m.Len())
	fmt.Println(m.Plen())
}