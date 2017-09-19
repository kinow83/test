package main
import (
	"os"
	"fmt"
	"io/ioutil"
	"bufio"
	"strings"
)

func main() {
	file, err := os.OpenFile("hello4.txt",
		os.O_CREATE|os.O_RDWR|os.O_TRUNC,
		os.FileMode(0644))

	if err != nil {
		fmt.Println(err)
		return
	}
	defer file.Close()

	n := 0
	s := "안녕하세요"
	n, err = file.Write([]byte(s))
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println(n, "바이트 저장 완료")

	fi, err := file.Stat()
	if err != nil {
		fmt.Println(err)
		return
	}

	var data = make([]byte, fi.Size())
	file.Seek(0, os.SEEK_SET)

	n, err = file.Read(data)
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println(n, "바이트 읽기 완료")
	fmt.Println(string(data))

	/**
	===========================================================
	 */

	s = "강석주 화이팅"
	err = ioutil.WriteFile("hello5.txt", []byte(s), os.FileMode(644))
	if err != nil {
		println(err)
		return
	}

	data, err = ioutil.ReadFile("hello5.txt")
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println(string(data))

	/**
	===========================================================
	 */
	file, err = os.OpenFile("hello6.txt",
		os.O_CREATE|os.O_RDWR|os.O_TRUNC,
		os.FileMode(0644))
	if err != nil {
		fmt.Println(err)
		return
	}
	defer file.Close()

	w := bufio.NewWriter(file)
	w.WriteString("6번째 문자열 기록중")
	w.Flush()

	r := bufio.NewReader(file)
	fi, _ = file.Stat()
	b := make([]byte, fi.Size())
	file.Seek(0, os.SEEK_SET)
	r.Read(b)
	fmt.Println(string(b))

	/**
	===========================================================
	 */
	file, err = os.OpenFile("hello7.txt",
		os.O_CREATE|os.O_RDWR|os.O_TRUNC,
		os.FileMode(0644))
	if err != nil {
		fmt.Println(err)
		return
	}
	defer file.Close()

	s = "모 이렇게 파일 읽고/쓰는 방법이 많냐??"
	rp := strings.NewReader(s)
	w = bufio.NewWriter(file)
	w.ReadFrom(rp)
	w.Flush()

	r = bufio.NewReader(file)
	fi, _ = file.Stat()
	file.Seek(0, os.SEEK_SET)
	b = make([]byte, fi.Size())
	n, _ = r.Read(b)
	fmt.Println(string(b))
}
