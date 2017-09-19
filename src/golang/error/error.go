package main
import (
	"time"
	"fmt"
	"log"
)

type NError struct {
	time time.Time
	value int
}

func (e NError) Error() string {
	return fmt.Sprintf("%v: Error Num:%d", e.time, e.value)
}

func hello(n int) (string, error) {
	if n == 1 {
		s := fmt.Sprintln("Hello", n)
		return s, nil
	}
	return "", NError{time.Now(), n}
}

func main() {
	s, err := hello(1)
	if err != nil {
		log.Fatal(err)
	}
	fmt.Println(s)

	s, err = hello(2)
	if err != nil {
		log.Fatal(err)
	}
	fmt.Println(s)
}
