package main
import (
	"fmt"
	"time"
)

func SWAP(a *int, b *int) {
	tmp := *a
	*a = *b
	*b = tmp
}
type Sort struct {
	_arr []int
	_len int
}
func (this *Sort) print(title string, duration time.Duration) {
	fmt.Println(title)
	for _,v:=range this._arr {
		fmt.Print(v, " ")
	}
	fmt.Println("[duration:", duration, "]")
}
func (this *Sort)SetArray(arr []int) {
	this._arr = make([]int, len(arr))
	this._len = len(arr)
	for i,v := range arr {
		this._arr[i] = v
	}
}
func (this *Sort)bubbleSort()  {
	for i:=0; i<this._len; i++ {
		for k:=0; k<this._len-i-1; k++ {
			if this._arr[k] > this._arr[k+1] {
				SWAP(&this._arr[k], &this._arr[k+1])
			}
		}
	}
}
func (this *Sort)insertSort() {
	for i:=1; i<this._len; i++ {
		in := this._arr[i]
		k := i-1
		for ; k>=0 && in<this._arr[k]; k-- {
			this._arr[k+1] = this._arr[k]
		}
		this._arr[k+1] = in
	}
}
func (this *Sort)SelectSort() {
	for i:=0; i<this._len; i++ {
		min := this._arr[i]
		idx := i
		for k:=i+1; k<this._len; k++ {
			if min > this._arr[k] {
				min = this._arr[k]
				idx = k
			}
		}
		this._arr[idx] = this._arr[i]
		this._arr[i] = min
	}
}
func (this *Sort)QuickSort() {
	this.quickSort(0, this._len-1)
}
func (this *Sort)quickSort(start int, end int) {
	if start >= end {
		return
	}
	mid  := (start+end)/2
	pivot:= this._arr[mid]

	SWAP(&this._arr[start], &this._arr[mid])
	p, q := start+1, end

	for {
		for ; this._arr[p] <= pivot; p++ {}
		for ; this._arr[q] >  pivot; q-- {}
		if p > q { break }
		SWAP(&this._arr[p], &this._arr[q])
	}
	SWAP(&this._arr[start], &this._arr[q])

	this.quickSort(start, q-1)
	this.quickSort(q+1, end)
}
func main() {
	sort := Sort{}
	sort.SetArray([]int{6,2,9,13,15,12,1,8,16,5,11,4,14,7,3,10})
	sort.print("initialize", time.Duration(0))

	start := time.Now()

	start = time.Now()
	sort.bubbleSort()
	sort.print("bubbleSort", time.Since(start))

	start = time.Now()
	sort.insertSort()
	sort.print("insertSort", time.Since(start))

	start = time.Now()
	sort.SelectSort()
	sort.print("SelectSort", time.Since(start))

	start = time.Now()
	sort.QuickSort()
	sort.print("QuickSort", time.Since(start))
}