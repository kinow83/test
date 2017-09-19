package main
import "fmt"

type Person struct {
	name string
	age int
}

func MergePerson(a, b Person) <-chan Person {
	out:=make(chan Person)
	go func() {
		out <- Person{a.name+"~"+b.name, a.age+b.age}
	}()
	return out
}
func main() {
	p1 := Person{"kinow", 33}
	p2 := Person{"maybe", 31}
	c := MergePerson(p1, p2)
	fmt.Println(<-c)
}