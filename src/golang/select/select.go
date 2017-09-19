package main
import (
	"time"
	"fmt"
)


func main() {
	c1 := make(chan int)
	c2 := make(chan string)

	go func() {
		for {
			c1 <- 10
			time.Sleep(100 * time.Millisecond)
		}
	}()

	go func() {
		for {
			c2 <- "hello"
			time.Sleep(500 * time.Millisecond)
		}
	}()

	go func() {
		for {
			select {
			case i := <- c1:
				fmt.Println("c1:", i)
			case s := <- c2:
				fmt.Println("c2:", s)
			case <- time.After(50 * time.Millisecond):
				fmt.Println("timeout")
			}
		}
	}()

	time.Sleep(5 * time.Second)
}
