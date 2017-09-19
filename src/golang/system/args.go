package main
import (
	"fmt"
	"os"
	"flag"
)

func main() {
	fmt.Println(os.Args)


	title   := flag.String("title", "파이란", "영화이름")
	runtime := flag.Int("runtime", 60, "상영시간")
	release := flag.Bool("release", false, "개봉여부")

	flag.Parse()
	n := flag.NFlag()
	fmt.Println("parsing : ", n)
	if n == 0 {
		flag.Usage()
		return
	}

	fmt.Printf("영화이름: %s\n상영시간: %d\n", *title, *runtime)

	if *release == true {
		fmt.Println("개봉여부: 개봉");
	} else {
		fmt.Println("개봉여부: 미개봉");
	}
}
