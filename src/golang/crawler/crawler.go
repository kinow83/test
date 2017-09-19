package main
import (
	"golang.org/x/net/html"
	"net/http"
	"log"
	"fmt"
	"runtime"
	"sync"
)

var fetched = struct{
	m map[string] error
	sync.Mutex
} {m: make(map[string]error)}

func fetch(url string) (*html.Node, error)  {
	res, err := http.Get(url)
	if err != nil {
		log.Println(err)
		return nil, err
	}

	doc, err := html.Parse(res.Body)
	if err != nil {
		log.Println(err)
		return nil, err
	}

	return doc, nil
}

func parseFollowing(doc *html.Node) []string {
	var urls = make([]string, 0)

	var f func(*html.Node)
	f = func(n *html.Node) {
		if n.Type == html.ElementNode && n.Data == "img" {
			for _, a := range n.Attr {

				// avatar
				if a.Key == "class" && a.Val == "avatar left" {
					for _, a:= range n.Attr {
						if a.Key == "alt" {
							fmt.Println(a.Val)
							break
						}
					}
				}

				// gravatar
				if a.Key == "class" && a.Val == "gravatar" {
					user := n.Parent.Attr[0].Val

					urls = append(urls, "https://github.com/"+user+"/following")
					break
				}
			}
		}
		for c:= n.FirstChild; c != nil; c = c.NextSibling {
			f(c)
		}
	}

	f(doc)

	return urls
}

func crawl(url string) {
	fetched.Lock()
	if _, ok := fetched.m[url]; ok {
		fetched.Unlock()
		return
	}
	fetched.Unlock()

	doc, err := fetch(url)

	fetched.Lock()
	fetched.m[url] = err
	fetched.Unlock()

	urls := parseFollowing(doc)

	done := make(chan bool)
	for _, u := range urls {
		go func(url string) {
			crawl(u)
			done <- true
		}(u)
	}

	for i:=0; i<len(url); i++ {
		<-done
	}

}

func parsingTest(n *html.Node) {

	if n == nil {
		return
	}

	fmt.Println(n.Type, n.Data, n.Attr)

	for c:= n.FirstChild; c != nil; c = c.NextSibling {
		parsingTest(c)
	}
}

func main() {
	runtime.GOMAXPROCS(runtime.NumCPU())

	crawl("https://github.com/kinow/following")

}
