// go build -o tcp tcp.go
// ./tcp

package main

import (
	"bufio"
	"fmt"
	"net"
	"time"
)

func startTCPServer() {
	listener, err := net.Listen("tcp", "127.0.0.1:9999")
	if err != nil {
		panic(err)
	}
	defer listener.Close()

	fmt.Println("TCP server started on 127.0.0.1:9999")

	for {
		conn, err := listener.Accept()
		if err != nil {
			fmt.Println("Accept error:", err)
			continue
		}
		go handleTCPConnection(conn)
	}
}

func handleTCPConnection(conn net.Conn) {
	defer conn.Close()

	reader := bufio.NewReader(conn)
	received := 0

	for {
		message, err := reader.ReadString('\n')
		if err != nil {
			fmt.Println("Read error:", err)
			return
		}

		received++
		fmt.Printf("Received (%d): %s", received, message)

		time.Sleep(10 * time.Millisecond)
	}
}

func startTCPClient() {
	time.Sleep(1 * time.Second)

	conn, err := net.Dial("tcp", "127.0.0.1:9999")
	if err != nil {
		panic(err)
	}
	defer conn.Close()

	total := 1000
	fmt.Println("Client sending", total, "messages...")

	for i := 0; i < total; i++ {
		message := fmt.Sprintf("Message %d\n", i)
		_, err := conn.Write([]byte(message))
		if err != nil {
			fmt.Println("Write error:", err)
			continue
		}
		time.Sleep(1 * time.Millisecond)
	}

	fmt.Println("Client done sending")
}

func main() {
	go startTCPServer()
	go startTCPClient()

	select {} // block forever
}
