package main
import (
	"sort"
	"fmt"
)


type Student struct {
	name string
	score float32
}

type By func(s1, s2 *Student) bool

func (by By) Sort(Students []Student) {
	sorter := &studentSoter{
		Students: Students,
		by      : by,
	}
	sort.Sort(sorter)
}


type studentSoter struct {
	Students []Student
	by       By
}

func (s *studentSoter) Len() int {
	return len(s.Students)
}

func (s *studentSoter) Less(i, j int) bool {
	return s.by(&s.Students[i], &s.Students[j])
}

func (s *studentSoter) Swap(i, j int) {
	s.Students[i], s.Students[j] = s.Students[j], s.Students[i]
}


func main() {
	s := []Student{
		{"ckinow", 99},
		{"akaka", 43},
		{"bkangsukju", 194},
	}

	name := func(n1, n2 *Student) bool {
		return n1.name < n2.name
	}

	score := func(s1, s2 *Student) bool {
		return s1.score < s2.score
	}

	reverseScore := func(s1, s2 *Student) bool {
		return !score(s1, s2)
	}

	fmt.Println("1.................\n", s)

	By(name).Sort(s)
	fmt.Println("2.................\n", s)

	By(score).Sort(s)
	fmt.Println("3.................\n", s)

	By(reverseScore).Sort(s)
	fmt.Println("4.................\n", s)

}

