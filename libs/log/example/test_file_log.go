package main

import(
	"github.com/SpiritDyn123/gocygame/libs/log"
	syslog"log"
	"sync"
)

func main() {
	logger, _:= log.New(log.CreateFileLog("filelog.log", 10000000), "debug", syslog.LstdFlags|syslog.Lshortfile, false)
	log.Export(logger)
	var wg sync.WaitGroup
	wg.Add(100)
	for i:=0;i < 100;i++ {
		go func(id int){
			defer wg.Done()
			for k:= 0; k < 1000;k++ {
				log.Debug("routine:%d line:%d", id, k+1)
			}
		}(i)
	}
	wg.Wait()
	log.Close()
}
