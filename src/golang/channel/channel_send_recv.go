package main
import (
	"fmt"
	"time"
)

func producer(c chan<- int)  {
	for i:=0; i<5; i++ {
		c <- i
	}
	c <- 100
//	fmt.Println(<-c) // error compile
}
func comsumer(c <-chan int)  {
	for i:= range c {
		fmt.Println(i)
	}
//	fmt.Println(<-c)
//	c <- 1 // error compile
}

func main() {
	c := make(chan  int)
	go comsumer(c)
	time.Sleep(5 * time.Second)
	go producer(c)
	fmt.Scanln()
}