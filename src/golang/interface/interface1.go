package main
import "fmt"

type hello interface  {
}

type MyInt int
func (this MyInt)()  {
	
}

func main() {
	var h hello
	fmt.Println(h)

	var i MyInt = 5
	fmt.Println(i)
}
