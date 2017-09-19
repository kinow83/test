package main
import (
	"os"
	"fmt"
	"bufio"
)


func main() {
	file, err := os.OpenFile("kinow1.txt", os.O_CREATE|os.O_RDWR|os.O_TRUNC, os.FileMode(0644))
	if err != nil {
		fmt.Println(err)
		return
	}
	defer file.Close()

	r := bufio.NewReader(file)
	w := bufio.NewWriter(file)

	rw := bufio.NewReadWriter(r, w)

	rw.WriteString("아 진짜 많다.~~~")
	rw.WriteString("\n")
	rw.WriteString("이젠 집에 가자!!")
	rw.Flush()

	file.Seek(0, os.SEEK_SET)
	for data,_,_ := rw.ReadLine();
		len(data) !=0;
		data,_,_ = rw.ReadLine() {
		fmt.Println(string(data))
	}
}