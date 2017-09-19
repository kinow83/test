package main
import (
	"reflect"
	"fmt"
)

type Person struct  {
	name string `tag1:"이름" tag2:"Name nAme naMe namE"`
	age int `tag1:"나이" tag2:"Age"`
}

func main() {
	var f float64 = 1.3

	t := reflect.TypeOf(f)
	v := reflect.ValueOf(f)

	fmt.Println(t)
	fmt.Println(v)
	fmt.Println(v.Float())


	p := Person{}
	name, ok := reflect.TypeOf(p).FieldByName("name")
	fmt.Println(ok, name.Tag.Get("tag1"), name.Tag.Get("tag2"))


	fmt.Println("................")
	var a *int = new(int)
	*a = 1
	fmt.Println(reflect.TypeOf(a))
	fmt.Println(reflect.ValueOf(a))
	fmt.Println(reflect.ValueOf(a).Elem())
	fmt.Println(reflect.ValueOf(a).Elem().Int())

	fmt.Println("................")
	var b interface{}
	b = 1
	fmt.Println(reflect.TypeOf(b))
	fmt.Println(reflect.ValueOf(b))
//	fmt.Println(reflect.ValueOf(b).Elem())
//	fmt.Println(reflect.ValueOf(b).Elem().Int())
}

