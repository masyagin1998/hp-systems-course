// go build -o udp udp.go
// ./udp

package main

import (
	"fmt"
	"net"
	"time"
)

func startUDPServer() {
	addr := net.UDPAddr{
		Port: 9999,
		IP:   net.ParseIP("127.0.0.1"),
	}
	conn, err := net.ListenUDP("udp", &addr)
	if err != nil {
		panic(err)
	}
	defer conn.Close()

	fmt.Println("Server started on", addr.String())

	buffer := make([]byte, 1024)
	received := 0

	for {
		n, remoteAddr, err := conn.ReadFromUDP(buffer)
		if err != nil {
			fmt.Println("Read error:", err)
			continue
		}

		message := string(buffer[:n])
		received++
		fmt.Printf("Received (%d): %s from %s\n", received, message, remoteAddr)

		time.Sleep(10 * time.Millisecond)
	}
}

func startUDPClient() {
	time.Sleep(1 * time.Second)

	serverAddr := net.UDPAddr{
		Port: 9999,
		IP:   net.ParseIP("127.0.0.1"),
	}
	conn, err := net.DialUDP("udp", nil, &serverAddr)
	if err != nil {
		panic(err)
	}
	defer conn.Close()

	total := 1000
	fmt.Println("Client sending", total, "messages...")

	for i := 0; i < total; i++ {
		message := fmt.Sprintf("Message %d", i)
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
	go startUDPServer()
	go startUDPClient()

	select {}
}
