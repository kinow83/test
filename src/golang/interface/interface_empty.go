package main
import (
	"reflect"
	"strconv"
	"fmt"
)

type Person struct {
	name string
	age int
}
type Animal struct {
	kind string
	name string
}

func formatString3(arg interface{}) string {
	switch arg.(type) {
	case Person:
		p := arg.(Person)
		return "instance "+ p.name +" "+strconv.Itoa(p.age)
	case *Person:
		p := arg.(*Person)
		return "pointer "+ p.name +" "+strconv.Itoa(p.age)
	default:
		return "Error  "+reflect.TypeOf(arg).String()
	}
}
func main() {
	fmt.Println(formatString3(Person{"kinow", 33}))
//	var i interface{} = Person{"kaka", 22}
//	fmt.Println(i.(Animal))
	var andrew = new(Person)
	andrew.name = "andrew"
	andrew.age = 22
	fmt.Println(formatString3(andrew))

	var kinow *Person
	kinow = new(Person)
	kinow.name = "kinow"
	kinow.age = 33
	fmt.Println(formatString3(kinow))

	pkinow := new(Person)
	pkinow.name = "kinow"
	pkinow.age = 33
	fmt.Println(formatString3(pkinow))
}