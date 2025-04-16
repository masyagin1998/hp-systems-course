// go build -o http http.go
// ./http

// curl -X POST http://localhost:8080/test -d "hello world" -H "X-Custom: echo-me"

package main

import (
	"fmt"
	"io"
	"net/http"
)

func echoHandler(w http.ResponseWriter, r *http.Request) {
	fmt.Printf("Received %s request for %s\n", r.Method, r.URL.Path)

	body, err := io.ReadAll(r.Body)
	if err != nil {
		http.Error(w, "Failed to read body", http.StatusInternalServerError)
		return
	}
	defer r.Body.Close()

	fmt.Fprintf(w, "Method: %s\n", r.Method)
	fmt.Fprintf(w, "URL: %s\n", r.URL.String())
	fmt.Fprintf(w, "Headers:\n")
	for name, values := range r.Header {
		for _, value := range values {
			fmt.Fprintf(w, "  %s: %s\n", name, value)
		}
	}
	fmt.Fprintf(w, "Body:\n%s\n", string(body))
}

func main() {
	http.HandleFunc("/", echoHandler)

	port := 8080
	fmt.Printf("HTTP Echo server listening on http://localhost:%d\n", port)
	err := http.ListenAndServe(fmt.Sprintf(":%d", port), nil)
	if err != nil {
		panic(err)
	}
}
