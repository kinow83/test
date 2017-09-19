package main
import (
	"os"
	"fmt"
	"unicode/utf8"
)

func main() {
	file1, _ := os.Create("hello1.txt")
	defer file1.Close()
	fmt.Fprint(file1, 1, 1.1, "Hello, World")

	file2, _ := os.Create("hello2.txt")
	defer file2.Close()
	fmt.Fprintln(file2, 1, 1.1, "Hello, World")

	file3, _ := os.Create("hello3.txt")
	defer file3.Close()
	fmt.Fprintf(file3, "%d,%f,%s", 1, 1.1, "Hello, World")

	s := "안녕하세요"
	fmt.Println(len(s))
	fmt.Println(utf8.RuneCountInString(s))

	b:=[]byte("안녕하세요")
	fmt.Println(b)



}
