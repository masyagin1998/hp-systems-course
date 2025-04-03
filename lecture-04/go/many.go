// ulimit -u
// go build many.go
// ./many

package main

import (
	"fmt"
	"runtime"
	"time"
)

const NumWorkers = 1_000_000
const LogEvery = 10_000

func worker(id int) {
    time.Sleep(time.Second)

	var counter uint64 = 0
	for {
		counter++
		if counter%LogEvery == 0 {
			fmt.Printf("Routine %d still running... (counter = %d)\n", id, counter)
		}
	}
}

func main() {
	runtime.GOMAXPROCS(runtime.NumCPU())

	fmt.Printf("Starting %d goroutines...\n", NumWorkers)
	for i := 0; i < NumWorkers; i++ {
		go worker(i)
		fmt.Printf("Started %d routine...\n", i + 1)
	}
    fmt.Printf("Started %d goroutines successfully\n", NumWorkers)

	time.Sleep(10 * time.Second)

	fmt.Println("Done")
}
