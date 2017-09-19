package main
import (
	"runtime"
	"time"
	"fmt"
	"sync"
)

func main() {
	runtime.GOMAXPROCS(runtime.NumCPU())
	var mutex = new(sync.Mutex)
	var data = []int{}

	go func() {
		for i:=0; i<1000; i++ {
			mutex.Lock()
			data = append(data, 1)
			mutex.Unlock()

			runtime.Gosched()
		}
	}()

	go func() {
		for i:=0; i<1000; i++ {
			mutex.Lock()
			data = append(data, 1)
			mutex.Unlock()

			runtime.Gosched()
		}
	}()

	time.Sleep(2 * time.Second)
	fmt.Println(len(data))
}