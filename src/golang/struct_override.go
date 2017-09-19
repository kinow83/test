package main
import "fmt"

type Person struct {
	name string
	age int
}
func (this *Person) greeting1(name string){
	this.name = name
	fmt.Println("Person")
}

type Student struct  {
	Person
	school string
	grade int
}
func (this *Student) greeting1(name string)  {
	this.name = name
	fmt.Println("Student1", this.school, this.grade, this.name, this.age)
}
func (this Student) greeting2(name string)  {
	this.name = name
	fmt.Println("Student2", this.school, this.grade, this.name, this.age)
}
func (this *Student) print() {
	fmt.Printf("===>  name:%s, age:%d, school:%s, grade:%d\n", this.name, this.age, this.school, this.grade)
}

func main() {
	var s Student
	s.name = "kaka"
	s.print()

	s.Person.greeting1("default")
	s.print()

	s.greeting1("ref")
	s.print()

	s.greeting2("instance")
	s.print()
}