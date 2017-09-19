package main
import "fmt"

func calc() func(x int) int {
	a, b := 3, 5
	return func(x int) int {
		return a*x + b
	}
}

func main(){
	sum := func(a, b int) int {
		return a+b
	}
	fmt.Println(sum(1,2))

	a, b := 3, 5
	f := func(x int) int {
		return a*x + b
	}
	fmt.Println(f(5))

	f = calc()
	fmt.Println(f(1))
	fmt.Println(f(2))
	fmt.Println(f(3))
	fmt.Println(f(4))
}