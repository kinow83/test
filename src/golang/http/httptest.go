package main
import "net/http"




func main() {
	http.HandleFunc("/", func(res http.ResponseWriter, req *http.Request) {
		res.Write([]byte("Hi~~ Man"))
	})

	http.ListenAndServe(":80", nil)
}
