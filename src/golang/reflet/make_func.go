package main
import (
	"reflect"
	. "fmt"
)


func h(args []reflect.Value) [] reflect.Value {
	Println("Hello, World")
	return nil
}

func sum(args []reflect.Value) []reflect.Value {
	a, b := args[0], args[1]
	if a.Kind() != b.Kind() {
		Println("타입이 다릅니다.")
		return nil
	}

	switch a.Kind() {
	case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
		return []reflect.Value
	case reflect.Uint, reflect.Uint8, reflect.Uint16, reflect.Uint32, reflect.Uint64:
	case reflect.Float32, reflect.Float64:
	case reflect.String:
	}
}

func main() {
	var hello func()

	fn := reflect.ValueOf(&hello).Elem()

	v := reflect.MakeFunc(fn.Type(), h)

	fn.Set(v)

	hello()


}