package main
import (
	"container/ring"
	"fmt"
)

func Ring()  {
	data := []string{"a", "b", "c", "d", "e"}

	r := ring.New(len(data))

	for i:=0; i<r.Len(); i++ {
		r.Value = data[i]
		r = r.Next()
	}

	r.Do(func(x interface{}) {
		fmt.Println(x, " ")
	})


	r = r.Move(1)
	fmt.Println("Cur : ", r.Value)
	fmt.Println("Prev: ", r.Prev().Value)
	fmt.Println("Next: ", r.Next().Value)

	r = r.Move(1)
	fmt.Println("Cur : ", r.Value)
	fmt.Println("Prev: ", r.Prev().Value)
	fmt.Println("Next: ", r.Next().Value)
}

func main() {
	Ring()
}
