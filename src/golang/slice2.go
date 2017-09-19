package main
import "fmt"
func main(){
	a4 := [3]int{1,2,3}
	var b4[3]int
	fmt.Println("a4:",a4, "\n", "b4:", b4)
	b4 = a4
	fmt.Println("a4:",a4, "\n", "b4:", b4)
	fmt.Println(a4 == b4)
	
}