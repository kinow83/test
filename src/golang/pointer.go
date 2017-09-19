package main
import "fmt"

func hello(n *int)  {
	*n = 2
}

func main() {
	var numPtr *int = new(int)
	*numPtr = 1
	*numPtr++
	fmt.Println(numPtr)
	fmt.Println(*numPtr)
	fmt.Println("")

	n := 1
	hello(&n)
	fmt.Println(n)
}