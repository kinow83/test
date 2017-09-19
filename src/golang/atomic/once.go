package main
import (
	"sync"
	"fmt"
	"runtime"
)

func hello() {
	println("Hello")
}

func main() {
	runtime.GOMAXPROCS(runtime.NumCPU())

	var once = new(sync.Once)

	for i:=0; i<10; i++ {
		go func(n int) {
			fmt.Println("goroutine : ", n)
			once.Do(hello)
		}(i)
	}

	fmt.Scanln()
}