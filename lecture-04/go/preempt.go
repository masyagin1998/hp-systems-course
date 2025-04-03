// go build preempt.go
// ./preempt
// GODEBUG=asyncpreemptoff=1 ./preempt

package main

import (
	"fmt"
	"time"
	"runtime"
)

func spin() {
    fmt.Println("Hello from spin")

	for {
	}
}

func main() {
	runtime.GOMAXPROCS(1)

	go spin()

	time.Sleep(5 * time.Second)
	fmt.Println("Hello from main")
}
