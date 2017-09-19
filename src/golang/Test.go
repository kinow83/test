package main

import "fmt"
import "unicode/utf8"


func sum(a []int) int {
	var s = 0
	for i:=0; i<len(a); i++ {
		s += a[i]
	}
	return s
}

func main()  {
	SS := 1

	fmt.Println(1,2,"3", SS)

	var num1 byte = 10
	var num2 byte = 0x32
	var num3 byte = 'a'
	fmt.Println(num1)
	fmt.Println(num2)
	fmt.Println(num3)


	var s1 = "한글"
	fmt.Println(len(s1))
	fmt.Println(utf8.RuneCountInString(s1))

	var s2 string = "한글"
	var s3 string = "Go"

	fmt.Println(s1 == s2)
	fmt.Println(s1 + s2 + s3)

	var a [5] int
	a[2] = 7
	fmt.Println(a)
}
