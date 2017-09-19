package main
import "fmt"

func sum(a int, b int) int {
	return a+b
}
func diff(a int, b int) int {
	return a-b
}
func SumAndDiff(a int, b int)(int, int) {
	return a+b, a-b
}
func SumandDiff2(a int, b int) (ret_sum int, ret_diff int) {
	ret_sum = a+b
	ret_diff = a-b
	return
}
func factorial(n uint64) uint64 {
	if n == 0 {
		return 1
	}
	return n * factorial(n-1)
}
func SumArg(n ... int) (total int) {
	fmt.Println("len n", len(n), cap(n))
	total = 0
	for _, v := range n {
		total += v
	}
	return
}
func main(){

	func() {
		fmt.Println("call anonymous function")
	}()

	func(s string) {
		fmt.Println(s)
	}("call anonymous function")

	var r2 int
	r2 = func(a int, b int) int {
		return a+b
	}(1,2)
	fmt.Println(r2)

	fmt.Println("function point")
	f := map[string]func(int, int) int {
		"sum" : sum,
		"diff" : diff,
	}
	fmt.Println(f["sum"](1,2))
	fmt.Println(f["diff"](1,2))

	fmt.Println("double return")
	sum1, diff1 := SumAndDiff(6,2)
	fmt.Println(sum1, diff1)
	_, diff2 := SumAndDiff(6,2)
	fmt.Println(diff2)
	sum3, _ := SumAndDiff(6,2)
	fmt.Println(sum3)

	fmt.Println(SumandDiff2(6, 2))

	fmt.Println("call factorial")
	fmt.Println(factorial(5))

	ff := []func(int, int) int {
		sum,
		diff,
	}
	fmt.Println(ff[0](1,2,))
	fmt.Println(ff[1](1,2,))

	var hello func(int, int) int = sum
	world := hello
	fmt.Println(hello(1,2))
	fmt.Println(world(1,2))

	fmt.Println(SumArg(1,2,3,4,5))

	n := []int{1,2,3,4,5}
	r := SumArg(n...)
	fmt.Println(r)
}
