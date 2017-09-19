package main
import (
	"fmt"
	"cmd/compile/internal/amd64"
)

func main() {
	var a []int = make([]int, 5)
	var b = make([]int, 5)
	c := make([]int, 5)

	fmt.Println(a)
	fmt.Println(b)
	fmt.Println(c)

	a1 := make([]int, 5, 10)
	fmt.Println(len(a1))
	fmt.Println(cap(a1))

	a2 := []int{1,2,3,4,5}
	fmt.Println(len(a2), cap(a2))
	a2 = append(a2, 6,7,8,9,10,11,12,13,14,15,16,17,18,19)
	fmt.Println(len(a2), cap(a2))

	a3 := []int{1,2,3,4,5}
	b3 := make([]int, 3)
	fmt.Println("a3:",a3, "\n", "b3:", b3)
	copy(b3, a3)
	fmt.Println("a3:",a3, "\n", "b3:", b3)
	b3[0] = 9
	fmt.Println("a3:",a3, "\n", "b3:", b3)

	fmt.Println("=======")

	a4 := [3]int{1,2,3}
	var b4[3]int
	fmt.Println("a4:",a4, "\n", "b4:", b4)
	b4 = a4
	fmt.Println("a4:",a4, "\n", "b4:", b4)

}