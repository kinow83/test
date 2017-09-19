package main
import (
	"fmt"
	"time"
	"sync"
)


type Runable interface {
	Run()
}

type Work1 int
type Work2 int

func (w *Work1) Run() {
	fmt.Println("Work1 run:",*w)
	*w++
}
func (w *Work2) Run() {
	fmt.Println("\tWork2 run:",*w)
	*w++
}

func main() {
	requst := make(chan Runable)
	var wg sync.WaitGroup

	wg.Add(20)

	go func() {
		w1 := Work1(0)
		for i:=0; i<10; i++ {
			requst <- &w1
			time.Sleep(1 * time.Second)
		}
	}()

	go func() {
		w2 := Work2(0)
		for i:=0; i<10; i++ {
			requst <- &w2
			time.Sleep(1 * time.Second)
		}
	}()


	go func() {
		fmt.Println("wait group")
		wg.Wait()
		fmt.Println("close request")
		close(requst)
	}()

	for q := range requst {
		q.Run()
		wg.Done()
	}

	fmt.Println("reach LOOP")
	WHAT:
	for {
		for i:=0; i<10; i++ {
			time.Sleep(1 * time.Second)
			fmt.Println("for ", i, "in LOOP")

			if i == 3 {
				fmt.Println("break LOOP")
				break WHAT
			}
		}
	}
}
