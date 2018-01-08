package utils

import (
	"os"
	"os/signal"
	"syscall"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"sync"
	"github.com/SpiritDyn123/gocygame/libs/chanrpc"
	"github.com/SpiritDyn123/gocygame/libs/timer"
	"github.com/SpiritDyn123/gocygame/libs/go"
	"sort"
)

type IService interface {
	GetName() string
	Start() error
	Close()
	Pool(cs chan bool)
	GetPriority() int
}

func Run(server IService) {
	RunMutli(server)
}

type sort_is []IService

func (ss sort_is) Len() int {
	return len(ss)
}

func (ss sort_is) Swap(i, j int) {
	ss[i], ss[j] = ss[j], ss[i]
}
func (ss sort_is) Less(i, j int) bool {
	return ss[i].GetPriority() < ss[j].GetPriority()
}

func RunMutli(sers ...IService) {
	sort.Sort(sort_is(sers))

	for _, server := range sers {
		log.Release("%s启动中", server.GetName())
		if err := server.Start(); err != nil {
			log.Release("%s启动失败,原因：%v", server.GetName(), err)
			return
		}
	}

	closeSig := make(chan bool)
	var wg sync.WaitGroup
	wg.Add(len(sers))
	for _, server := range sers {
		go func(cs chan bool) {
			defer wg.Done()
			server.Pool(cs)
		}(closeSig)
		log.Release("%s启动成功", server.GetName())
	}

	chanSig := make(chan os.Signal)
	signal.Notify(chanSig, os.Interrupt, syscall.SIGTERM)
	sig := <- chanSig

	for _, server := range sers {
		log.Release("%s关闭中（signal:%v）", server.GetName(), sig)
		closeSig<-true
		server.Close()
	}
	wg.Wait()

	log.Release("关闭成功")
}

//提供一个默认简单的pooller
type Pooller struct {
	ChanServer *chanrpc.Server //主循环管道
	TimerServer *timer.Dispatcher //定时器
	GoServer *g.Go //异步go协程
}

func (pl *Pooller) Pool(cs chan bool) {
	for {
		select {
			case <- cs:
				pl.ChanServer.Close()
				pl.GoServer.Close()
				return
			case ci := <- pl.ChanServer.ChanCall:
				pl.ChanServer.Exec(ci)
			case ct := <- pl.TimerServer.ChanTimer:
				ct.Cb()
			case cc := <- pl.GoServer.ChanCb:
				pl.GoServer.Cb(cc)
		}
	}
}

