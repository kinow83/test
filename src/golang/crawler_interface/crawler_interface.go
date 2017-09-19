package main
import (
	"sync"
	"golang.org/x/net/html"
	"net/http"
	"log"
	"fmt"
	"strings"
	"runtime"
)
//////////////////////////////////
// 팔로잉 저장
//////////////////////////////////
type FollowingResult struct {
	url string
	name string
}
//////////////////////////////////
// 별표 저장소 저장
//////////////////////////////////
type StarsResult struct {
	repo string
}
//////////////////////////////////
// 중복 URL 처리
//////////////////////////////////
type FetchedUrl struct {
	m map[string]error
	sync.Mutex
}
//////////////////////////////////
// 중복 저장소 처리
//////////////////////////////////
type FetchedRepo struct {
	m map[string]struct{}
	sync.Mutex
}
//////////////////////////////////
// Crawler 인터페이스
//////////////////////////////////
type Crawler interface {
	Crawl()
	Parse(doc *html.Node) <-chan string
	Request(url string)
}
//////////////////////////////////
// 팔로잉 수집 구조체 정의
//////////////////////////////////
type GitHubFollowing struct {
	fetchedUrl *FetchedUrl
	p          *Pipeline
	stars      *GitHubStars
	result     chan FollowingResult
	url        string
}
func (g *GitHubFollowing) Request(url string) {
	g.p.requst <- &GitHubFollowing{
		fetchedUrl: g.fetchedUrl,
		p:          g.p,
		result:     g.result,
		stars:      g.stars,
		url:        url,
	}
}
func (g *GitHubFollowing) Parse(doc *html.Node) <-chan string {
	name := make(chan string)

	go func() {
		var f func(*html.Node)
		f = func(n *html.Node) {

			if n.Type == html.ElementNode && n.Data == "img" {
				for _, a := range n.Attr {
					if a.Key =="class" {
						if a.Val == "avatar left" {
							for _, a := range n.Attr {
								if a.Key == "alt" {
									i := strings.Index(a.Val, "@")
									name <- a.Val[i+1:]
									return
								}
							}
						}
						if a.Val == "gravatar" {
							user := n.Parent.Attr[0].Val
							// 팔로잉 URL을 파이프라인으로 보냄
							g.Request("https://github.com/"+user+"/following")
							// 별표 URL을 파이프라인으로 보냄
							g.stars.Request("https://github.com/stars/"+user)
							break
						}
					}
				}
			}
			for c := n.FirstChild; c != nil; c = c.NextSibling {
				f(c)
			}
		}
		f(doc)
	}()

	return name
}
func (g *GitHubFollowing) Crawl() {
	g.fetchedUrl.Lock()
	if _, ok := g.fetchedUrl.m[g.url]; ok {
		g.fetchedUrl.Unlock()
		return
	}
	g.fetchedUrl.Unlock()

	doc, err := fetch(g.url)
	if err != nil {
		go func(u string) {
			g.Request(u)
		}(g.url)
		return
	}

	g.fetchedUrl.Lock()
	g.fetchedUrl.m[g.url] = err
	g.fetchedUrl.Unlock()

	name := <-g.Parse(doc)
	g.result <- FollowingResult{
		url:  g.url,
		name: name, // channel
	}
}
//////////////////////////////////
// 별표 저장소 수집 구조체 정의
//////////////////////////////////
type GitHubStars struct {
	fetchedUrl  *FetchedUrl
	fetchedRepo *FetchedRepo
	p           *Pipeline
	result      chan StarsResult
	url         string
}
func (g *GitHubStars) Request(url string)  {
	g.p.requst <- &GitHubStars{
		fetchedUrl:  g.fetchedUrl,
		fetchedRepo: g.fetchedRepo,
		p:           g.p,
		result:      g.result,
		url:         url,
	}
}
func (g *GitHubStars) Parse(doc *html.Node) <-chan string{
	repo := make(chan string)

	go func() {
		defer close(repo)

		var f func(*html.Node)
		f = func(n *html.Node) {
			if n.Type == html.ElementNode && n.Data == "span" {
				for _, a := range n.Attr {
					if a.Key == "class" && a.Val == "prefix" {
						repo <- n.Parent.Attr[0].Val
						break
					}
				}
			}
			for c := n.FirstChild; c != nil; c = c.NextSibling {
				f(c)
			}
		}
		f(doc)
	}()

	return repo
}
func (g *GitHubStars) Crawl() {
	g.fetchedUrl.Lock()
	if _, ok := g.fetchedUrl.m[g.url]; ok {
		g.fetchedUrl.Unlock()
		return
	}
	g.fetchedUrl.Unlock()

	doc, err := fetch(g.url)
	if err != nil {
		go func(u string) {
			g.Request(u)
		}(g.url)
		return
	}

	g.fetchedUrl.Lock()
	g.fetchedUrl.m[g.url] = err
	g.fetchedUrl.Unlock()

	repositories := g.Parse(doc)
	for r := range repositories {
		g.fetchedRepo.Lock()
		if _, ok := g.fetchedRepo.m[r]; !ok {
			g.result <- StarsResult{r}
			g.fetchedRepo.m[r] = struct{}{}
		}
		g.fetchedRepo.Unlock()
	}
}
/////////////////////////////////////////////////
// Crawler 인터페이스 처리를 위한 파이프라인 구조체
/////////////////////////////////////////////////
type Pipeline struct {
	requst chan Crawler // Crawler 인터페이스 채널
	done   chan struct{}
	wg     *sync.WaitGroup
}
func NewPipeline() *Pipeline {
	return &Pipeline{
		requst: make(chan Crawler),
		done:   make(chan struct{}),
		wg:     new(sync.WaitGroup),
	}
}
func (p *Pipeline) Worker() {
	for r := range p.requst {
		select {
		case <-p.done:
			return
		default:
			r.Crawl()
		}
	}
}
func (p *Pipeline) Run() {
	const numWorkers = 10
	p.wg.Add(numWorkers)

	for i:=0; i<numWorkers; i++ {
		go func() {
			p.Worker()
			p.wg.Done()
		}()
	}

	go func() {
		p.wg.Wait()
	}()
}

func fetch(url string) (*html.Node, error) {
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

func main() {
	runtime.GOMAXPROCS(runtime.NumCPU())

	p := NewPipeline()
	p.Run()

	stars := &GitHubStars{
		fetchedUrl:  &FetchedUrl{m: make(map[string]error)},
		fetchedRepo: &FetchedRepo{m:make(map[string]struct{})},
		p:           p,
		result:      make(chan StarsResult),
	}
	following := &GitHubFollowing{
		fetchedUrl: &FetchedUrl{m: make(map[string]error)},
		p:          p,
		stars:      stars,
		result:     make(chan FollowingResult),
		url:        "https://github.com/pyrasis/following",
	}

	p.requst <- following

	count := 0

	LOOP:
	for {
		select {
		case f := <- following.result:
			fmt.Println("url: ",f.url)
			fmt.Println("\tfollowing:",f.name)
		case s := <- stars.result:
			fmt.Println("stars:",s.repo)
			if count == 1000 {
				close(p.done)
				break LOOP
			}
			count++
		}
	}

}
