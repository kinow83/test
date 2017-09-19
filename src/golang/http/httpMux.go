package main
import "net/http"


func main() {
	mux := http.NewServeMux()

	mux.HandleFunc("/", func(res http.ResponseWriter, req *http.Request) {
		res.Write([]byte("aaaaaaaaaaaaaaa"))
	})

	http.ListenAndServe(":8080", mux)
}