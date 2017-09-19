package main
import (
	"encoding/json"
	"fmt"
)

func main() {
	data := make(map[string]interface{})

	data["name"] = "wow"
	data["age"] = 99

	doc, err := json.Marshal(data)
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println(string(doc))

	doc, err = json.MarshalIndent(data, "", "\t")
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println(string(doc))
}
