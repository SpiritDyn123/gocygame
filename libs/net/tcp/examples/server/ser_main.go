package main

import (
	"github.com/SpiritDyn123/gocygame/libs/utils"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/libs/chanrpc"
	"github.com/SpiritDyn123/gocygame/libs/go"
	"github.com/SpiritDyn123/gocygame/libs/timer"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp/examples/common"
	bb"encoding/binary"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp/examples/protocol/binary"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"time"
	"fmt"
	"flag"
)

var (
	tls bool
)

type server struct {
	utils.Pooller
	ser *tcp.Server
	sessionMap map[uint64]*tcp.Session
}

func(ser *server) GetName() string {
	return "测试服务器"
}

func(ser *server) Start() (err error) {
	if ser.sessionMap == nil {
		ser.sessionMap = make(map[uint64]*tcp.Session)
	}

	ser.ChanServer = chanrpc.NewServer(100)
	ser.ChanServer.Register(common.ACCEPT_KEY, ser.onAccept)
	ser.ChanServer.Register(common.RECV_KEY, ser.onRecv)
	ser.ChanServer.Register(common.CLOSED_KEY, ser.onClose)

	ser.GoServer = g.New(100)

	ser.TimerServer = timer.NewDispatcher(100)
	ser.TimerServer.AfterFunc(time.Second, ser.onTimer)

	//p := codec.FixLen(&binary.BinaryProtocl{}, common.MSGLEN, common.BytesOrder, common.MAX_RECV, common.MAX_SEND)
	codec := &binary.BinaryCodec{}
	mp := &tcp.MsgParser{}
	mp.SetByteOrder(common.BytesOrder == bb.LittleEndian)
	mp.SetMsgLen(common.MSGLEN, common.MSGLEN, common.MAX_RECV)
	if tls {
		ser.ser, err = tcp.CreateTLSServer("tcp", common.TCP_ADDR, "./tls_cert.pem", "./tls_key.pem", codec, mp, common.SEND_CHAN_SIZE, ser.ChanServer, common.ACCEPT_KEY, common.RECV_KEY, common.CLOSED_KEY)
		log.Release("run tls tcp server")
	} else {
		ser.ser, err = tcp.CreateServer("tcp", common.TCP_ADDR, codec, mp, common.SEND_CHAN_SIZE, ser.ChanServer, common.ACCEPT_KEY, common.RECV_KEY, common.CLOSED_KEY)
		log.Release("run tcp server")
	}

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
	session, ok := args[0].(*tcp.Session)
	if !ok {
		log.Error("onAccept %v args[0] is not session", args)
		return
	}

	if _, ok := ser.sessionMap[session.Id()]; ok {
		log.Error("onAccept session:%d exsited", session.Id())
		return
	}

	ser.sessionMap[session.Id()] = session
	log.Release("session:%d accepted", session.Id())
}

func (ser *server) onRecv(args []interface{}) {
	session, ok := args[0].(*tcp.Session)
	if !ok {
		log.Error("onRecv %v args[0] is not session", args)
		return
	}

	if _, ok := ser.sessionMap[session.Id()]; !ok {
		log.Error("onRecved session:%d not exsited", session.Id())
		return
	}

	log.Release("session:%d recv:%s", session.Id(), string(args[1].([]byte)))
	session.Send([]byte(fmt.Sprintf("hello session %d i has recved you msg", session.Id())))
}

func (ser *server) onClose(args []interface{}) {
	session, ok := args[0].(*tcp.Session)
	if !ok {
		log.Error("onClose %v args[0] is not session", args)
		return
	}

	delete(ser.sessionMap, session.Id())
	log.Release("session:%d closed", session.Id())
}

func (ser *server) onTimer() {
	log.Release("cur connections num:%d", len(ser.sessionMap))
	time.AfterFunc(time.Second * 2, ser.onTimer)
}

func main() {
	flag.BoolVar(&tls, "tls", false, "use tls or not")
	flag.Parse()

	utils.RunMutli(&server{})
}

