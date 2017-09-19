package main
import (
	"os"
	"fmt"
	"bufio"
)


func main() {
	file, _ := os.Open("e:\\sync.txt")
	defer file.Close()
	m := make(map[int]int)
	var n1, n2 int
	r := bufio.NewReaderSize(file, 1024)
	line, isPrefix, err := r.ReadLine()
	for err == nil && !isPrefix {
		s := string(line)
		//fmt.Println(s)
		fmt.Sscanf(s, "%d %d", &n1, &n2)
		fmt.Println(n2)
		m[n2]++
		line, isPrefix, err = r.ReadLine()
	}

	for i, v := range m {
		if v > 1 {
			fmt.Println("====>", i, v)
		}
	}
}