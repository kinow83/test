package main
import (
	"container/list"
	"fmt"
	"container/heap"
	"reflect"
)


//////////////////////////////////////////////////////
// List
//////////////////////////////////////////////////////
func List()  {
	l := list.New()
	l.PushBack(10)
	l.PushBack("a")
	l.PushBack(30)

	fmt.Println("Front : ", l.Front().Value)
	fmt.Println("Back  : ", l.Back().Value)

	for e:=l.Front(); e!=nil; e=e.Next() {
		fmt.Print(e.Value, " ")
	}
}

//////////////////////////////////////////////////////
// Heap
//////////////////////////////////////////////////////

type MinHeap []int

func (h *MinHeap) Less(i, j int) bool {
	r := (*h)[i] < (*h)[j]
	fmt.Printf("Less %d < %d %t\n", (*h)[i], (*h)[j], r)
	return r
}

func (h *MinHeap) Swap(i, j int) {
	fmt.Printf("Swap %d %d\n", (*h)[i], (*h)[j])
	(*h)[i], (*h)[j] = (*h)[j], (*h)[i]
}

func (h *MinHeap) Len() int {
	return len(*h)
}


func (h *MinHeap) Push(x interface{}) {
	fmt.Println("Push", x)
	*h = append(*h, x.(int))
}

func (h *MinHeap) Pop() interface{} {
	old := *h
	n := len(old)
	x := old[n-1]
	*h = old[0:n-1]
	return x
}

func HeapTree()  {
	data := new(MinHeap)
	fmt.Println(reflect.TypeOf(data))
	heap.Init(data)

	heap.Push(data, 5)
	heap.Push(data, 2)
	heap.Push(data, 7)
	heap.Push(data, 3)

	fmt.Println(data, (*data)[0], (*data)[len(*data)-1])
}

func main() {
	//List()
	HeapTree()
}