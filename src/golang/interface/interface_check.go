package main
import (
	"fmt"
	"reflect"
	"strconv"
)

type Duck struct {
}
func (this Duck) quack() {
	fmt.Println("꽥")
}
func (this Duck) feathers() {
	fmt.Println("오리는 흰색과 회색 털을 가지고 있다.")
}
func (this *Duck) walking() {
	fmt.Println("뒤뚱뒤뚱")
}
func (this Duck) talk() string {
	return "I'm Duck"
}

type Person struct {
}
func (this Person) quack() {
	fmt.Println("사람은 오리를 흉내냅니다. 꽥")
}
func (this Person) feathers() {
	fmt.Println("사람은 땅에서 깃털을 주워서 보여줍니다.")
}
func (this Person) walking() {
	fmt.Println("터벅터벅")
}
func (this Person) talk() string {
	return "I'm Person"
}

type Quackable interface {
	quack()
	feathers()
	talk() string
}
type walkingable interface {
	walking()
}

func CheckQuackableInterface(i interface{}) bool {
	_, ok := i.(Quackable)
	return ok
}
func inTheForest(q Quackable) {
	q.quack()
	q.feathers()
}

func formatString(arg interface{}) string {
	switch arg.(type) {
	case int:
		i := arg.(int)
		return strconv.Itoa(i)
	case float32:
		f := arg.(float32)
		return strconv.FormatFloat(float64(f), 'f', -1, 32)
	case float64:
		f := arg.(float64)
		return strconv.FormatFloat(f, 'f', -1, 64)
	case string:
		s := arg.(string)
		return s
	case Quackable:
		q := arg.(Quackable)
		return q.talk()
	default:
		return "Error "+ reflect.TypeOf(arg).String()
	}
}

func main() {
	var donald Duck
	var kinow Person

	fmt.Println(CheckQuackableInterface(donald))
	fmt.Println(CheckQuackableInterface(kinow))

	var empty interface{}
	empty = donald
	fmt.Println(empty)
	fmt.Println(reflect.TypeOf(empty))

	fmt.Println("------------------")
	inTheForest(donald)
	inTheForest(kinow)

	fmt.Println("------------------")

	fmt.Println(formatString(kinow))
	fmt.Println(formatString(1))
	fmt.Println(formatString(2.5))
	fmt.Println(formatString("kaka wow"))
}