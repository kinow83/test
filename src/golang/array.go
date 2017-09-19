package main
import "fmt"

func main() {
	var a[5]int
	a[2] = 7
	fmt.Println(a)

	var a1[5]int = [5]int{32,29,23,55,21}
	var b1 = [5]int{32,29,23,55,21}
	c1 := [5]int{32,29,23,55,21}

	fmt.Println(a1)
	fmt.Println(b1)
	fmt.Println(c1)

	a2 := [...]int{32,29,23,55,21}
	b2 := [...]string{"maria","andrew", "john"}
	fmt.Println(a2)
	fmt.Println(b2)

	a3 := [...]int{32,29,23,55,21}
	b3 := a3
	c3 := [...]int{32,29,23,55,21}
	fmt.Println("a3", a3)
	fmt.Println("b3", b3)
	b3[2] = 3
	fmt.Println("a3 == b3 ? ", a3 == b3)
	fmt.Println("a3 == c3 ? ", a3 == c3)

	for index, value := range c3 {
		fmt.Println(index, "=", value)
	}
	for value := range c3 {
		fmt.Println("   ", value)
	}
	for _, value := range c3 {
		fmt.Println("   ", "   ", value)
	}
	a4 := [][]int {
		{1,2,3},
		{4,5,6},
		{7,8,9},
	}
	fmt.Println(a4)
}
