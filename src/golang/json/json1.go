package main
import (
	"encoding/json"
	"fmt"
)

func main() {
	doc := `
	{
		"name": "kinow",
		"age": 33
	}
	`

	var data map[string] interface{}

	json.Unmarshal([]byte(doc), &data)

	fmt.Println("name:", data["name"])
	fmt.Println("age:", data["age"])
	fmt.Println("school:", data["school"])
}
