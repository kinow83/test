package main
import "fmt"

type MyInt int
func (this MyInt) Print() {
	fmt.Println(this)
}
func (this MyInt) inc() {
	this++
}

type Rectangle struct  {
	width, height int
}
func (this *Rectangle) Print() {
	fmt.Println(this.width, this.height)
}
func (this *Rectangle) inc() {
	this.width++
	this.height++
}

type Printer interface {
	Print()
	inc()
}

func main() {
	i := MyInt(5)
	r := Rectangle{10, 20}
	var p Printer

	p = i
	p.Print()
	fmt.Println(p)
	p.inc()
	fmt.Println(i)

	fmt.Println("____________")

	p = &r
	p.Print()
	fmt.Println(p)
	p.inc()
	fmt.Println(r)

	fmt.Println("____________")

	var ii MyInt = 5
	rr := Rectangle{10, 20}

	pArr := []Printer{ii, &rr}
	for index, _ := range pArr {
		pArr[index].inc()
		pArr[index].Print()
	}
	for _, value := range pArr {
		value.inc()
		value.Print()
	}
}