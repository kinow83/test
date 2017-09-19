package main
import "fmt"

func f(){
	defer func() {
		s := recover()
		fmt.Println("...", s)
	}()

	panic("Error!!!")
}
func f2()  {
	defer func() {
		s := recover()
		fmt.Println("f2...", s)
	}()

	a := [...]int{1,2}
	for i:=0; i<5; i++ {
		fmt.Println(a[i])
	}
}
func main() {
	/*
	panic("Error!")
	fmt.Println("Hello world")
	*/

	/*
	f()
	fmt.Println("Hello world")
	*/

	f2()
	fmt.Println("Hello world")
}