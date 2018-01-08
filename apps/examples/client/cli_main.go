package main

import (
	"github.com/SpiritDyn123/gocygame/libs/utils"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"time"
	"github.com/SpiritDyn123/gocygame/apps/examples/common"
	"github.com/SpiritDyn123/gocygame/libs/chanrpc"
	"github.com/funny/link/codec"
	"github.com/SpiritDyn123/gocygame/apps/examples/protocol/binary"
	"github.com/funny/link"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"github.com/name5566/leaf/timer"
	"fmt"
)

type client struct {
	cli *tcp.Client
	chanSer *chanrpc.Server
	timeDisp *timer.Dispatcher
	sessionMap map[uint64]*link.Session

	tmpSend uint64
}

func (c *client) GetName() string {
	return "测试客户端"
}

func (c *client) Start() (err error) {
	c.sessionMap = make(map[uint64]*link.Session)
	c.tmpSend = 0

	c.chanSer = chanrpc.NewServer(100)
	c.chanSer.Register(common.ACCEPT_KEY, c.onAccept)
	c.chanSer.Register(common.CLOSED_KEY, c.onClose)
	c.chanSer.Register(common.RECV_KEY, c.onRecv)

	c.timeDisp = timer.NewDispatcher(100)
	c.timeDisp.AfterFunc(time.Second, c.onTimer)
	p := codec.FixLen(&binary.BinaryProtocl{}, common.MSGLEN, common.BytesOrder, common.MAX_SEND, common.MAX_RECV)
	c.cli = &tcp.Client{
		AutoReconnect:true,
		ConnectInterval:time.Second,

		Network:"tcp",
		Addr:"127.0.0.1"+common.TCP_ADDR,
		DialTimeOut:time.Second* 10,
		ConnectNum:10,
		Protocol:p,
		SendChanSize:common.SEND_CHAN_SIZE,

		ChanServer:c.chanSer,
		ConnectKey:common.ACCEPT_KEY,
		ClosedKey:common.CLOSED_KEY,
		RecvKey:common.RECV_KEY,
	}
	c.cli.Connect()
	return
}

func (c *client) Close() {
	c.cli.Stop()
}

func (c *client) Pool(cs chan bool) {
	for {
		select {
		case _ = <- cs:
			c.chanSer.Close()
			return
		case ci := <- c.chanSer.ChanCall:
			c.chanSer.Exec(ci)

		case tc := <- c.timeDisp.ChanTimer:
			tc.Cb()
		}
	}
}

func (c *client) GetPriority() int {
	return 0
}

func (c *client) onAccept(args []interface{}) {
	session, ok := args[0].(*link.Session)
	if !ok {
		log.Error("onAccept %v args[0] is not session", args)
		return
	}

	if _, ok := c.sessionMap[session.ID()]; ok {
		log.Error("onAccept session:%d exsited", session.ID())
		return
	}

	c.sessionMap[session.ID()] = session
	log.Release("session:%d accepted", session.ID())
}

func (c *client) onRecv(args []interface{}) {
	session, ok := args[0].(*link.Session)
	if !ok {
		log.Error("onRecv %v args[0] is not session", args)
		return
	}

	if _, ok := c.sessionMap[session.ID()]; !ok {
		log.Error("onRecved session:%d not exsited", session.ID())
		return
	}

	log.Release("session:%d recv:%s", session.ID(), args[1].(string))
}

func (c *client) onClose(args []interface{}) {
	session, ok := args[0].(*link.Session)
	if !ok {
		log.Error("onClose %v args[0] is not session", args)
		return
	}

	delete(c.sessionMap, session.ID())
	log.Release("session:%d closed", session.ID())
}

func (c *client) onTimer() {
	log.Release("cur connections num:%d", len(c.sessionMap))
	if  len(c.sessionMap) >0 {
		maxSessionId := uint64(0)
		for id, _ := range c.sessionMap {
			if id > maxSessionId {
				maxSessionId = id
			}
		}

		for {
			if session, ok := c.sessionMap[c.tmpSend];ok {
				log.Release("session %d sended", c.tmpSend)
				session.Send(fmt.Sprintf("hello sb server, i am session_%d", c.tmpSend))
				c.tmpSend++
				break
			} else {
				if c.tmpSend > maxSessionId {
					c.tmpSend = 0
				} else {
					c.tmpSend++
				}
			}
		}
	}

	time.AfterFunc(time.Second * 2, c.onTimer)
}

func main() {

	utils.RunMutli(&client{})
}
