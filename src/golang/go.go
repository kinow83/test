package main
import (
	"fmt"
	"math/rand"
	"time"
	"runtime"
)

func hello() {
	fmt.Println("hello")
}
func hellon(n int) {
	r := rand.Intn(100)
	time.Sleep(time.Duration(r))
	fmt.Println(n)
}

func main() {
	for i:=0; i<100; i++ {
		go hellon(i)
	}
	fmt.Printf("...%d\n", runtime.NumCPU())
	fmt.Scanln()
}