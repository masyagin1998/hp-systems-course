// go build -o echo echo.go
// ./echo

package main

import (
	"bufio"
	"fmt"
	"net"
	"strings"
	"time"
)

func startServer(address string) {
	listener, err := net.Listen("tcp", address)
	if err != nil {
		panic(err)
	}
	fmt.Println("Server started at", address)

	for {
		conn, err := listener.Accept()
		if err != nil {
			fmt.Println("Accept error:", err)
			continue
		}
		go handleConnection(conn)
	}
}

func handleConnection(conn net.Conn) {
	defer conn.Close()
	reader := bufio.NewReader(conn)

	for {
		message, err := reader.ReadString('\n')
		if err != nil {
			fmt.Println("Read error:", err)
			return
		}
		message = strings.TrimSpace(message)
		fmt.Println("Received:", message)

		conn.Write([]byte(message + "\n"))
	}
}

func startClient(address string) {
	conn, err := net.Dial("tcp", address)
	if err != nil {
		panic(err)
	}
	defer conn.Close()

	reader := bufio.NewReader(conn)

	for i := 1; ; i++ {
		message := fmt.Sprintf("Hello %d", i)
		_, err := fmt.Fprintf(conn, message+"\n")
		if err != nil {
			fmt.Println("Write error:", err)
			return
		}

		response, err := reader.ReadString('\n')
		if err != nil {
			fmt.Println("Read error:", err)
			return
		}
		fmt.Println("Echoed:", strings.TrimSpace(response))
		time.Sleep(1 * time.Second)
	}
}

func main() {
	address := "localhost:9999"
	go startServer(address)
	time.Sleep(500 * time.Millisecond)
	startClient(address)
}
