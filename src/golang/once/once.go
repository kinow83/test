package main
import (
	"fmt"
	"sync"
)

func whatever(any interface{}) {
	fmt.Println(any)
}


func main() {
	once := new(sync.Once)
	for i:=0; i<3; i++ {
		go func(n int) {
			fmt.Println("goroutin : ", n)
			once.Do(func() {
				fmt.Println("what??")
			})
		}(i)
	}

	whatever("what2")

	fmt.Scanln()
}
