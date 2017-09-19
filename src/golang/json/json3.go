package main
import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
)

type Author struct {
	Name string `json:"name"`
	Email string `json:"e~~~mail"`
	address string
}

type Comment struct {
	Id uint64
	Author Author
	Content string
}

type Article struct {
	Id uint64
	Title string
	Author Author
	Content string
	Recommends []string
	Comments []Comment
}

func unmarshal() {
	doc := `
	[{
		"Id": 1,
		"Title": "Hellow, World!",
		"Author": {
			"Name": "Maria",
			"Email": "maria@naver.com",
			"address": "Seoul"
		},
		"Content": "Hello~",
		"Recommends": [
			"John",
			"Andrew"
		],
		"Comments":[{
			"Id": 1,
			"Author": {
				"Name": "Andrew",
				"Email": "andrew@naver.com"
			},
			"Content": "Hello Maria"
		}]
	}]
	`



	var data []Article
	err := json.Unmarshal([]byte(doc), &data)
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println(data)
}

func marshal() {
	data := make([]Article, 1)

	data[0].Id = 1
	data[0].Title = "Hello, World"
	data[0].Author.Name = "kinow"
	data[0].Author.Email = "kinow@naver.com"
	data[0].Author.address = "Seoul"
	data[0].Content = "Hello~"
	data[0].Recommends = []string{"John", "Andrew"}

	doc, _ := json.Marshal(data)
	fmt.Println(string(doc))

	doc, _ = json.MarshalIndent(data, "", " ")
	fmt.Println(string(doc))


	err := ioutil.WriteFile("Article.json", doc, os.FileMode(644))
	if err != nil {
		fmt.Println(err)
		return
	}
}

func ReadJson() {
	b, err := ioutil.ReadFile("Article.json")
	if err != nil {
		fmt.Println(err)
		return
	}

	var data []Article

	json.Unmarshal(b, &data)

	fmt.Println(data)
}

func main() {

	unmarshal()
	marshal()
	fmt.Println(".............................")
	ReadJson()
}
