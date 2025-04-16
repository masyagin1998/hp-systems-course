// go build -o tcp tcp.go
// ./tcp

package main

import (
	"bufio"
	"fmt"
	"io"
	"net"
	"strings"
)

func handleConnection(conn net.Conn) {
	defer conn.Close()

	reader := bufio.NewReader(conn)

	requestLine, err := reader.ReadString('\n')
	if err != nil {
		fmt.Println("Failed to read request line:", err)
		return
	}
	method, path, _, ok := parseRequestLine(requestLine)
	if !ok {
		fmt.Fprint(conn, "HTTP/1.1 400 Bad Request\r\n\r\nInvalid request line")
		return
	}

	fmt.Printf("Received %s request for %s\n", method, path)

	headers := make(map[string]string)
	var contentLength int
	for {
		line, err := reader.ReadString('\n')
		if err != nil {
			fmt.Println("Error reading header:", err)
			return
		}
		line = strings.TrimSpace(line)
		if line == "" {
			break
		}
		parts := strings.SplitN(line, ":", 2)
		if len(parts) == 2 {
			key := strings.TrimSpace(parts[0])
			value := strings.TrimSpace(parts[1])
			headers[key] = value
			if strings.ToLower(key) == "content-length" {
				fmt.Sscanf(value, "%d", &contentLength)
			}
		}
	}

	body := make([]byte, contentLength)
	if contentLength > 0 {
		_, err = io.ReadFull(reader, body)
		if err != nil {
			fmt.Println("Error reading body:", err)
			return
		}
	}

	fmt.Fprint(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n")
	fmt.Fprintf(conn, "Method: %s\n", method)
	fmt.Fprintf(conn, "URL: %s\n", path)
	fmt.Fprintf(conn, "Headers:\n")
	for k, v := range headers {
		fmt.Fprintf(conn, "  %s: %s\n", k, v)
	}
	fmt.Fprintf(conn, "Body:\n%s\n", string(body))
}

func parseRequestLine(line string) (method, path, version string, ok bool) {
	parts := strings.Fields(line)
	if len(parts) != 3 {
		return "", "", "", false
	}
	return parts[0], parts[1], parts[2], true
}

func main() {
	listener, err := net.Listen("tcp", "127.0.0.1:8080")
	if err != nil {
		panic(err)
	}
	defer listener.Close()

	fmt.Println("Raw TCP HTTP Echo server listening on http://localhost:8080")

	for {
		conn, err := listener.Accept()
		if err != nil {
			fmt.Println("Accept error:", err)
			continue
		}
		go handleConnection(conn)
	}
}
