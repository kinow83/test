package main
import (
	"fmt"
	"time"
)

func main() {
	c1 := make(chan int)
	c2 := make(chan string)

	go func() {
		for {
			i := <- c1
			fmt.Println("recv c1 : ", i)
			time.Sleep(100 * time.Millisecond)
		}
	}()

	go func() {
		for {
			c2 <- "Hellow World"
			time.Sleep(500 * time.Millisecond)
		}
	}()

	go func() {
		for {
			select {
			case c1 <- 10:
			case s := <-c2:
				fmt.Println("recv c2 : ", s)
			}
		}
	}()

	time.Sleep(6 * time.Second)
}