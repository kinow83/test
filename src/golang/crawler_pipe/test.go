package main
import (
	"net/http"
	"golang.org/x/net/html"
	"fmt"
)


func ViweHtml(doc *html.Node)  {
	for c:= doc.FirstChild; c!=nil; c=c.NextSibling {
		fmt.Println("type:",c.Type)
		for _,a := range c.Attr {
			fmt.Printf("\t%s=%s\n", a.Key, a.Val)
		}
		ViweHtml(c)
	}
}


func main() {


	url := "http://unet.co.kr"

	res,err := http.Get(url)
	if err != nil {
		println(err)
		return
	}
	doc,err:= html.Parse(res.Body)
	if err != nil {
		println(err)
		return
	}


	ViweHtml(doc)


}
