package main

import f "fmt"
import . "fmt"
import "runtime"

import (
	_ "unicode"
	_ "io/ioutil"
)

func multi_return(a int, b int) (int, int, int)  {
	return a, b, a+b
}

func main()  {
	f.Println("aaaaaaa")
	Println("aaaaaaa")
	Println("CPU Count: ", runtime.NumCPU())
/*
	var a, b, c int
	a, b, c = multi_return(12, 2)
	Println(a, b, c)
*/
	/*
	var b[] byte
	var err error

	if b, err = ioutil.ReadFile("c:\\ftconfig.ini"); err == nil {
		Println(b)
	} else {
		Println(err)
	}
	*/

	var i = 0
	for i < 5 {
		Println(i)
		i++
	}

	for i, j := 0, 0; i < 10; i, j = i+1, j+2 {
		Println(i, j)
	}

}