package main
import (
	"fmt"
	"time"
	"runtime"
)


func main() {
	runtime.GOMAXPROCS(1)
	c1 := make(chan int)

	go func() {
		for {
			i := <-c1
			fmt.Println("[1] c1 : ", i)
			time.Sleep(100 * time.Millisecond)
		}
	}()
	go func() {
		for {
			c1 <- 20
			time.Sleep(200 * time.Millisecond)
		}
	}()
	go func() {
		for {
			select {
			case c1 <- 10:
			case i := <-c1:
				fmt.Println("[2] c1 : ", i)
			case i := <-c1:
				fmt.Println("[3] c1 : ", i)
			}
		}
	}()

	time.Sleep(10 * time.Second)
}