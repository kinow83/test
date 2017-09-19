package main
import (
	"time"
	"container/list"
	"github.com/googollee/go-socket.io"
	"log"
	"net/http"
	"fmt"
)


type Event struct {
	EvtType   string
	User      string
	Timestamp int
	Text      string
}

type Subscription struct  {
	Archve []Event
	New    <-chan Event
}
func (s Subscription) Cancle() {
	g_unsubscribe <- s.New
	for {
		select {
		case _, ok := <- s.New:
			if !ok {
				return
			}
		default:
			return
		}
	}
}

var (
	g_subscribe   = make(chan (chan Subscription), 10)
	g_unsubscribe = make(chan (<-chan Event), 10)
	g_publish     = make(chan Event, 10)
)

func NewEvent(eventType, user, msg string) Event {
	return Event{eventType, user, int(time.Now().Unix()), msg}
}

/////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////
func Subscribe() Subscription {
	c := make(chan Subscription)
	g_subscribe <- c
	return <-c
}

func Join(user string)  {
	g_publish <- NewEvent("join", user, "")
}
func Say(user, msg string) {
	g_publish <- NewEvent("message", user, msg)
}
func Leave(user string) {
	g_publish <- NewEvent("leave", user, "")
}


func Chatroom()  {
	archive     := list.New()
	subscribers := list.New()

	for {
		select {
		case c := <- g_subscribe:
			var events[]Event
			for e := archive.Front(); e != nil; e = e.Next() {
				events = append(events, e.Value.(Event))
			}
			subscriber := make(chan Event, 10)
			subscribers.PushBack(subscriber)
			fmt.Println("join new user, total subscribers is ", subscribers.Len())

			c <- Subscription{events, subscriber}

		case event := <- g_publish:
			for e := subscribers.Front(); e != nil; e = e.Next() {
				subscriber := e.Value.(chan Event)
				fmt.Println("send msg to channel ", subscriber)
				subscriber <- event
			}
			if archive.Len() >= 20 {
				archive.Remove(archive.Front())
			}
			archive.PushBack(event)
			fmt.Println("New Event:", event);

		case c := <- g_unsubscribe:
			for e := subscribers.Front(); e != nil; e = e.Next() {
				subscriber := e.Value.(chan Event)

				if subscriber == c {
					subscribers.Remove(e)
					close(subscriber)
					break
				}
			}

		}
	}
}

func main() {
	server, err := socketio.NewServer(nil)
	if err != nil {
		log.Fatal(err)
	}

	go Chatroom()

	server.On("connection", func(so socketio.Socket) {
		fmt.Println("connection")
		sub := Subscribe()

		Join(so.Id())

		for _, event := range sub.Archve {
			so.Emit("event", event)
		}

		newMsg := make(chan string)

		so.On("message", func(msg string) {
			newMsg <- msg
		})

		so.On("disconnect", func() {
			Leave(so.Id())
			sub.Cancle()
		})

		go func() {
			for {
				select {
				case m := <- newMsg:
					Say(so.Id(), m)
				case e := <- sub.New:
					fmt.Println("Event Emit: ", e)
					so.Emit("event", e)
				}
			}
		}()
	})

	http.Handle("/socket.io/", server)
	http.Handle("/", http.FileServer(http.Dir(".")))
	err = http.ListenAndServe(":80", nil)
	if err != nil {
		log.Fatal(err)
	}
}
