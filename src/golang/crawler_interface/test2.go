package main
import (
	"fmt"
	"time"
)


type ChannelData struct  {
	name string
	text string
}


func main() {
	mainCh := make(chan (chan ChannelData))
	ch1 := make(chan ChannelData)
	defer 	close(mainCh)
	defer 	close(ch1)

	go func() {
		for {
			select {
			case <-mainCh:
				fmt.Println("recv mainCh")
			}
		}
	}()

	fmt.Println("gogo")
	go func() {
		time.Sleep(2 * time.Second)
		mainCh <- ChannelData{"1", "1"}
	}()

	fmt.Scanln()

}