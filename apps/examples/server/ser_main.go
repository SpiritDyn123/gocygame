package main

import (
	"github.com/SpiritDyn123/gocygame/libs/utils"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/libs/chanrpc"
	"github.com/SpiritDyn123/gocygame/libs/go"
	"github.com/SpiritDyn123/gocygame/libs/timer"
	"github.com/SpiritDyn123/gocygame/apps/examples/common"
	"github.com/funny/link/codec"
	"github.com/SpiritDyn123/gocygame/apps/examples/protocol/binary"
	"github.com/funny/link"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"time"
	"fmt"
)


type server struct {
	utils.Pooller
	ser *tcp.Server
	sessionMap map[uint64]*link.Session
}


func(ser *server) GetName() string {
	return "测试服务器"
}

func(ser *server) Start() (err error) {
	if ser.sessionMap == nil {
		ser.sessionMap = make(map[uint64]*link.Session)
	}

	ser.ChanServer = chanrpc.NewServer(100)
	ser.ChanServer.Register(common.ACCEPT_KEY, ser.onAccept)
	ser.ChanServer.Register(common.RECV_KEY, ser.onRecv)
	ser.ChanServer.Register(common.CLOSED_KEY, ser.onClose)

	ser.GoServer = g.New(100)

	ser.TimerServer = timer.NewDispatcher(100)
	ser.TimerServer.AfterFunc(time.Second, ser.onTimer)

	p := codec.FixLen(&binary.BinaryProtocl{}, common.MSGLEN, common.BytesOrder, common.MAX_RECV, common.MAX_SEND)
	ser.ser, err = tcp.CreateServer("tcp", common.TCP_ADDR, p, common.SEND_CHAN_SIZE, ser.ChanServer, common.ACCEPT_KEY, common.RECV_KEY, common.CLOSED_KEY)
	if err == nil {
		ser.ser.Start()
	}
	return
}

func(ser *server) Close() {
	ser.ser.Stop()
}


func(ser *server) Pool(cs chan bool) {
	ser.Pooller.Pool(cs)
}


func(ser *server)GetPriority() int {
 	return 0
}


func (ser *server) onAccept(args []interface{}) {
	session, ok := args[0].(*link.Session)
	if !ok {
		log.Error("onAccept %v args[0] is not session", args)
		return
	}

	if _, ok := ser.sessionMap[session.ID()]; ok {
		log.Error("onAccept session:%d exsited", session.ID())
		return
	}

	ser.sessionMap[session.ID()] = session
	log.Release("session:%d accepted", session.ID())
}

func (ser *server) onRecv(args []interface{}) {
	session, ok := args[0].(*link.Session)
	if !ok {
		log.Error("onRecv %v args[0] is not session", args)
		return
	}

	if _, ok := ser.sessionMap[session.ID()]; !ok {
		log.Error("onRecved session:%d not exsited", session.ID())
		return
	}

	log.Release("session:%d recv:%s", session.ID(), args[1].(string))
	session.Send(fmt.Sprintf("hello session %d i has recved you msg", session.ID()))
}

func (ser *server) onClose(args []interface{}) {
	session, ok := args[0].(*link.Session)
	if !ok {
		log.Error("onClose %v args[0] is not session", args)
		return
	}

	delete(ser.sessionMap, session.ID())
	log.Release("session:%d closed", session.ID())
}

func (ser *server) onTimer() {
	log.Release("cur connections num:%d", len(ser.sessionMap))
	time.AfterFunc(time.Second * 2, ser.onTimer)
}

func main() {
	utils.RunMutli(&server{})
}

