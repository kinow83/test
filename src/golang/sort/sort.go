package main
import (
	"sort"
	"fmt"
)

type Student struct {
	name string
	score float32
}

type Students []Student

func (s Students) Len() int {
	return len(s)
}

func (s Students) Less(i, j int) bool {
	return s[i].name < s[j].name
}

func (s Students) Swap(i, j int) {
	s[i], s[j] = s[j], s[i]
}

type ByScore struct {
	Students
}

func (s ByScore) Less(i, j int) bool {
	return s.Students[i].score < s.Students[j].score
}


func main() {
	s := Students{
		{"ckinow", 99},
		{"akaka", 43},
		{"bkangsukju", 194},
	}
	b := ByScore{s}

	fmt.Println("---------------")
	fmt.Println(s)
	fmt.Println(b)

	fmt.Println("1---------------")
	sort.Sort(s)
	fmt.Println(s)

	fmt.Println("2---------------")
	sort.Sort(b)
	fmt.Println(b)

	fmt.Println("3---------------")
	sort.Sort(sort.Reverse(ByScore{s}))
	fmt.Println(s)
}
