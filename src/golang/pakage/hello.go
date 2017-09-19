package main

import "fmt"
import "calc"


func main() {
	n, err := fmt.Println(calc.Sum(1,2))
	fmt.Println(n, err)
}
