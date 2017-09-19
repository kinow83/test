package main
import (
	"fmt"
	"time"
)

func main() {
	done := make(chan bool)
	count := 3

	go func() {
		for i:=0; i<count; i++ {
			fmt.Println("before go totine : ", i)
			done <- true
			fmt.Println("after go totine : ", i)
			time.Sleep(time.Second)
		}
	}()

	for i:=0; i<count; i++ {
		fmt.Println("before main : ", i)
		<-done
		fmt.Println("after main : ", i)
	}
}
