package main
import "fmt"

type Person struct  {
	name string
	age int
}

func (this *Person) greeting()  {
	fmt.Println("Person Hello")
}

type Student struct  {
	Person
	p Person
	school string
	grade int
}

func main() {
	var s Student
	s.p.greeting()

	ss := Student{Person{"kinow1", 20 }, Person{"kinow2", 30 }, "aa", 10}
	fmt.Println(ss)

	s.greeting()
	s.Person.greeting()
}