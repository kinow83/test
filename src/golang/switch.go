package main
import (
	"math/rand"
	"time"
	"fmt"
)

func main() {
	s := "hello"
	i := 2

	switch i {
	case 1:
		fmt.Println(1)
		break
	case 2:
		if s == "hello" {
			fmt.Println("hello 2")
			break
		}
		fmt.Println(2)
	}

	i = 8
	switch {
	case i >= 5 && i < 10:
		fmt.Println("i >= 5 && i < 10")
	case i >= 0 && i < 5:
		fmt.Println("i >= 0 && i < 5")
	}

	i = 4

	switch i {
	case 4:
		fmt.Println("4")
		fallthrough
	case 3:
		fmt.Println("3")
//		fallthrough
	case 2:
		fmt.Println("2")
	}

	fmt.Print("")

	rand.Seed(time.Now().UnixNano())
	switch i := rand.Intn(10); {
	case i >= 3 && i > 6:
		fmt.Println("i >= 3 && i > 6")
	case i == 9:
		fmt.Println(9)
	default:
		fmt.Println(i)
	}

	i = 3
	switch i {
	case 2, 4, 6:
		fmt.Println("짝수")
	case 1, 3, 5:
		fmt.Println("홀수")
	}
}
