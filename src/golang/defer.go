package main
import (
	"fmt"
	"os"
)

func hello() {
	fmt.Println("hello")
}
func world() {
	fmt.Println("world")
}
func HelloWorld() {
	defer func() {
		fmt.Println("world")
	}()
	func() {
		fmt.Println("hello")
	}()
}
func ReadHello() {
	file, err := os.Open("hello.txt")
	defer file.Close()

	if err != nil {
		fmt.Println(err)
		return
	}
	n := 0
	buf := make([]byte, 100)
	if n, err = file.Read(buf); err != nil {
		fmt.Println(err)
		return
	}

	fmt.Println("byte: ", n)
	fmt.Println(buf)
	fmt.Println(string(buf))
}

func main() {
	/*
	defer world()
	hello()
	hello()
	hello()
	*/
	HelloWorld()

	for i:=0; i<5; i++ {
		defer fmt.Printf("%d", i) // 4 3 2 1 0
	}

	ReadHello()
}