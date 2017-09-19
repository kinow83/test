package main
import (
	"fmt"
	"time"
)

func num(a, b int) <-chan int {
	out := make(chan int, 3)
	go func() {
		fmt.Println("ready")
		out <- a
		fmt.Println("<-a")
		out <- b
		fmt.Println("<-b")
		close(out)
		fmt.Println("<-close")
	}()
	return out
}
func sum(c <-chan int) <-chan int {
	out := make(chan int)
	go func() {
		r := 0
		for i := range c {
			fmt.Println("->", i)
			r += i
		}
		out <- r
	}()
	return out
}

func main() {
	fmt.Println("start")
	c := num(1, 2)
	time.Sleep(5 * time.Second)
	out := sum(c)
	fmt.Println(<-out)
}