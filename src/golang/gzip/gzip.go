package main
import (
	"os"
	"fmt"
	"compress/gzip"
	"io/ioutil"
)

func compress()  {
	file, err := os.OpenFile("kaka.txt.gz", os.O_CREATE|os.O_RDWR|os.O_TRUNC, os.FileMode(644))
	if err != nil {
		fmt.Println(err)
		return
	}
	defer file.Close()

	w := gzip.NewWriter(file)
	defer w.Close()

	s := "동해물과 백두산이 마르고 닳도록 하느님이 보우하사 우리나라만세~~"
	w.Write([]byte(s))
	w.Flush()
}

func uncompress() {
	file, err := os.OpenFile("kaka.txt.gz",
		os.O_RDONLY,
		os.FileMode(644))
	if err != nil {
		fmt.Println(err)
		return
	}
	defer file.Close()

	r, err := gzip.NewReader(file)
	r.Close()

	b, err := ioutil.ReadAll(r)
	fmt.Println(string(b))
}

func main() {

	fmt.Println("Do compress")
	compress()

	fmt.Println("Do uncompress")
	uncompress()
}
