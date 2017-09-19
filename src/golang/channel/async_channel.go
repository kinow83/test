package main
import (
	"runtime"
	"fmt"
)

func main() {
	runtime.GOMAXPROCS(1)

	done := make(chan bool, 3) // buffer 2
	c := 4

	go func() {
		for i:=0; i<c; i++ {
			done <- true
			fmt.Println("go routine: ", i)
		}
	} ()

	for i:=0; i<c; i++ {
		<- done
		fmt.Println("main: ", i)
	}

	// check close for channel
	fmt.Println("===check close for channel")
	ch := make(chan int, 1)
	go func() {
		ch <- 1
	}()
	a, ok := <-ch
	fmt.Println(a, ok)

	close(ch)
	a, ok = <-ch
	fmt.Println(a, ok)

	fmt.Println("===close and range channel")
	ch = make(chan int)
	go func() {
		for i:=0; i<5; i++ {
			ch <- 1
		}
		//close(ch)
	}()
	_, v := <-ch
	fmt.Println(v)
	for i:=range ch {
		fmt.Println("catch channel data")
		fmt.Println(i)
		fmt.Println("waiting next channel data")
	}
	fmt.Println("end")
	_, v = <-ch
	fmt.Println(v)
}
