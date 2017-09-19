package main
import (
	"time"
	"fmt"
)


var C int


func rch() <-chan string {
	s := make(chan string)

	go func() {
		fmt.Println("Call go func")
		time.Sleep(5 * time.Second)
		s <- fmt.Sprintf("%d", C)
		C++
	}()

	fmt.Println("return s channel")
	return s
}


func main() {

	fmt.Println("before rch")
	s := <-rch()
	fmt.Println("after  rch")

	fmt.Println(s)
}