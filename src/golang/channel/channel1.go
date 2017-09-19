package main
import "fmt"

func sum(a int, b int, c chan int) {
//	fmt.Println("call sum channel")
	c <- a + b
}

func main() {
	c := make(chan int)
	for i:=0; i<50; i++ {
		go sum(1, 2, c)
		n := <-c
		fmt.Println(n)
	}
	fmt.Println("-----------------")
}
