package main
import "fmt"

func main() {
	var s1, s2 int
	//n, _ := fmt.Scan(&s1, &s2)
	n, _ := fmt.Scanf("%d,%d", &s1, &s2)
	fmt.Println("입력개수 : ", n)
	fmt.Println(s1, s2)
}
