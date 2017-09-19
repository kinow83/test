package main
import "fmt"

type Rect struct  {
	width int
	height int
}

func main() {
	var rect1 *Rect
	rect1 = new(Rect)
	rect1.width = 10
	rect2 := new(Rect)

	fmt.Println(rect1)
	fmt.Println(*rect2)

	fmt.Println(rect1 == rect2)

	var r1 Rect = Rect{10, 20}
	r2 := Rect{45, 22}
	r3 := Rect{width: 30, height: 15}

	fmt.Println(r1)
	fmt.Println(r2)
	fmt.Println(r3)
}