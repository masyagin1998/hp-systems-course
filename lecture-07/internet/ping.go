// go mod init ping
// go get golang.org/x/net/icmp
// go get golang.org/x/net/ipv4
// go build -o ping ping.go
// ping 8.8.8.8
// sudo ./ping 8.8.8.8

package main

import (
	"encoding/binary"
	"fmt"
	"golang.org/x/net/icmp"
	"golang.org/x/net/ipv4"
	"log"
	"net"
	"os"
	"time"
)

func main() {
	if len(os.Args) < 2 {
		fmt.Fprintf(os.Stderr, "Usage: %s <IP_ADDRESS>\n", os.Args[0])
		os.Exit(1)
	}
	targetIP := os.Args[1]
	dest := net.ParseIP(targetIP)
	if dest == nil {
		log.Fatalf("Invalid IP address: %s", targetIP)
	}

	conn, err := net.ListenIP("ip4:icmp", &net.IPAddr{IP: net.IPv4zero})
	if err != nil {
		log.Fatalf("Error creating IP connection: %v", err)
	}
	defer conn.Close()

	pconn := ipv4.NewPacketConn(conn)
	if err := pconn.SetControlMessage(ipv4.FlagTTL, true); err != nil {
		log.Fatalf("Failed to set control message: %v", err)
	}

	pid := os.Getpid() & 0xffff
	seq := 0

	fmt.Printf("PING %s:\n", targetIP)

	for {
		seq++

		payload := make([]byte, 32)
		now := time.Now()
		binary.BigEndian.PutUint64(payload, uint64(now.UnixNano()))
		for i := 8; i < len(payload); i++ {
			payload[i] = 0xA5
		}

		echo := &icmp.Echo{
			ID:   pid,
			Seq:  seq,
			Data: payload,
		}
		message := &icmp.Message{
			Type: ipv4.ICMPTypeEcho,
			Code: 0,
			Body: echo,
		}
		msgBytes, err := message.Marshal(nil)
		if err != nil {
			log.Printf("Error marshalling ICMP message: %v", err)
			continue
		}

		_, err = pconn.WriteTo(msgBytes, nil, &net.IPAddr{IP: dest})
		if err != nil {
			log.Printf("Error sending packet: %v", err)
			continue
		}
		if err := conn.SetReadDeadline(time.Now().Add(1 * time.Second)); err != nil {
			log.Printf("Error setting read deadline: %v", err)
		}

		buf := make([]byte, 1500)
		for {
			n, cm, src, err := pconn.ReadFrom(buf)
			if err != nil {
				fmt.Printf("Request timed out for icmp_seq %d\n", seq)
				break
			}

			if n < 20+8 {
				continue
			}

			ipHeaderLen := int(buf[0]&0x0F) * 4

			recvMsg, err := icmp.ParseMessage(ipv4.ICMPTypeEchoReply.Protocol(), buf[ipHeaderLen:n])
			if err != nil {
				log.Printf("Error parsing ICMP message: %v", err)
				continue
			}
			if recvMsg.Type != ipv4.ICMPTypeEchoReply {
				continue
			}

			echoReply, ok := recvMsg.Body.(*icmp.Echo)
			if !ok || echoReply.ID != pid {
				continue
			}

			if echoReply.Seq != seq {
				continue
			}

			if len(echoReply.Data) < 8 {
				log.Printf("Received data too short")
				continue
			}
			sentTimeNano := int64(binary.BigEndian.Uint64(echoReply.Data[:8]))
			rtt := time.Since(time.Unix(0, sentTimeNano))

			ttl := 0
			if cm != nil {
				ttl = cm.TTL
			}

			fmt.Printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
				n-ipHeaderLen, src.String(), echoReply.Seq, ttl, float64(rtt.Nanoseconds())/1e6)
			break
		}

		time.Sleep(1 * time.Second)
	}
}
