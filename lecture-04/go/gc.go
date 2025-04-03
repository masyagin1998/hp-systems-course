// go build gc.go
// ./gc
// GOGC=1 ./gc
// GOGC=100 ./gc
// GOGC=1000 ./gc
// ...

package main

import (
	"fmt"
	"runtime"
	"time"
)

const (
	numObjects     = 1_000_000
	allocSizeBytes = 100
	sleepBetween   = 1 * time.Second
)

func main() {
	var mem runtime.MemStats

	for i := 0; ; i++ {
		data := make([][]byte, numObjects)
		for j := 0; j < numObjects; j++ {
			data[j] = make([]byte, allocSizeBytes)
		}

		runtime.ReadMemStats(&mem)
		fmt.Printf("[Iteration %d] HeapAlloc: %.2f MB, TotalAlloc: %.2f MB, NumGC: %d, PauseTotal: %.2f ms\n",
			i,
			float64(mem.HeapAlloc)/1024/1024,
			float64(mem.TotalAlloc)/1024/1024,
			mem.NumGC,
			float64(mem.PauseTotalNs)/1e6,
		)

		time.Sleep(sleepBetween)
	}
}
