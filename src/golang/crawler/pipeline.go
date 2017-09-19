package main
import (
	"fmt"
	"sync"
	"time"
	"runtime"
)

func worker(n int, done <-chan struct{}, jobs <-chan int, c chan<- string) {

	for j := range jobs {
		select {
		case c <- fmt.Sprintf("Worker: %d, Job: %d", n, j):
		case <- done:
			return
		}
	}
}


func inout(on bool, s chan string) bool {

	switch on {
	case true:
		s <- fmt.Sprint(true)
	case false:
		fmt.Println(<-s)
	}
	return !on
}

func main() {

if false {
	runtime.GOMAXPROCS(1)
	jobs := make(chan int)
	done := make(chan struct{})
	c := make(chan string)

	var wg sync.WaitGroup
	const numWorkers = 5

	wg.Add(numWorkers)

	for i := 0; i < numWorkers; i++ {
		go func(n int) {
			worker(n, done, jobs, c)
			wg.Done()
		}(i)
	}

	go func() {
		wg.Wait()
		close(c)
	}()

	go func() {
		for i := 0; i < 10; i++ {
			jobs <- i
			time.Sleep(10 * time.Millisecond)
		}
		close(done)
		close(jobs)
	}()

	for r := range c {
		fmt.Println(r)
	}
}

	on := true
	sc := make(chan string, 3)

	for i:=0; i<10; i++ {
		on = inout(on, sc)
	}
}