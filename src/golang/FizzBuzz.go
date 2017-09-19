package main
import "fmt"
func main(){
	for i:=0; i<101; i++ {
		switch {
		case i%3 == 0 && i%5 == 0:
			fmt.Print("FizzBuzz")
		case i%3 == 0:
			fmt.Println("Fizz")
		case i%5 == 0:
			fmt.Println("Buzz")
		default:
			fmt.Println(i)
		}
	}
}
