package main
import "fmt"

func main() {
	var s1, s2, s3 string

	s1 = fmt.Sprint(1, 1.1, "Kinow")
	fmt.Println(s1)

	s2 = fmt.Sprintln(1, 1.1, "Kinow")
	fmt.Println(s2)

	s3 = fmt.Sprintf("%d %f %s\n", 1, 1.1, "Kinow")
	fmt.Println(s3)


	var num1 int
	var num2 float32
	var s string

	input1 := "1\n1.1\nHello"
	n, _ := fmt.Sscan(input1, &num1, &num2, &s)

	fmt.Println("입력개수:",n)
	fmt.Println(num1, num2, s)

	num1 = 0
	num2 = 0
	s = ""

	input2 := "1 1.1 Hello"
	n, _ = fmt.Sscanln(input2, &num1, &num2, &s)
	fmt.Println("입력개수:",n)
	fmt.Println(num1, num2, s)

	num1 = 0
	num2 = 0
	s = ""

	input3 := "1,1.1,Hello World"
	n, _ = fmt.Sscanf(input3, "%d,%f,%s", &num1, &num2, &s)
	fmt.Println("입력개수:",n)
	fmt.Println(num1, num2, s)
}
