package main
import "fmt"

// copy values
func slice_copy(){
	a := []int{1,2,3}
	var b[]int
	b=a
	b[0]=9
//	fmt.Println(a==b)
	fmt.Println(a, len(a), cap(a))
	fmt.Println(b, len(b), cap(b))
}

// call by reference
func slice_ref(){
	a := [3]int{1,2,3}
	var b[3]int
	b=a
	b[0]=9
	fmt.Println(a==b)
	fmt.Println(a, len(a), cap(a))
	fmt.Println(b, len(b), cap(b))
}

// call by reference
func slice_index(){
	fmt.Println("slice_index")
	a := []int{1,2,3,4,5}
	b := a[0:2]
	b[0]=9
	fmt.Println(a, len(a), cap(a))
	fmt.Println(b, len(b), cap(b))
}

func slice_subindex(){
	fmt.Println("slice_subindex")
	a := []int{1,2,3,4,5,6,7,8}
	b := a[0:6:8]
	b[0] = 99
	fmt.Println(a, len(a), cap(a))
	fmt.Println(b, len(b), cap(b))
}

func main(){
	slice_copy()

	slice_ref()

	slice_index()

	slice_subindex()
}