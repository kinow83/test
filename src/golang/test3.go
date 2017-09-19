package main
import "fmt"


func Eratosthenes1(arr []int) {
	n := len(arr)
	for i:=2; i<n; i++ {
		for k:=2; k<n; k++ {
			if arr[k] != i && arr[k] % i == 0 {
				arr[k] = 0
			}
		}
	}
}

func Eratosthenes2(arr []int) {
	n := len(arr)
	for i:=2; i<n; i++ {
		if arr[i] == 0 {
			continue
		}
		for k := i+i; k<n; k+=i {
			arr[k] = 0
		}
	}
}

func main() {
	n := 5000
	arr := make([]int, n+1)

	for i:=1; i<=n; i++ {
		arr[i] = i
	}
	//fmt.Println(arr)

	Eratosthenes2(arr)

	for i:=2; i<=n; i++ {
		if arr[i] != 0 {
			fmt.Print(arr[i], " ")
		}
	}
}