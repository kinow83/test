package main
import "fmt"

func main(){
	const Iam = 0

	const (
		sun = iota
		mon = iota
		tue = iota
		wed = iota
		thu = iota
		fri = iota
		sat = iota
		numofdays
	)

	fmt.Println("thu is ", thu)
	fmt.Println("numofdays is ", numofdays)

	const (
		a, a1, a2 = 1 << iota, 1 << iota, 1 << iota
		b, b1 = 1 << iota, 1 << iota
		c = 1 << iota
		d = 1 << iota
	)

	fmt.Println(a, a1, a2, b, b1, c, d)

	const (
		bit0, mask0 = 1 << iota, 1 << iota - 1
//		bit1, mask1
		_, _
		_, _
		bit3, mask3
	)
	fmt.Println(bit0, bit3)

}