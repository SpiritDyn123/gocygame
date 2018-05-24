package tcp

import (
	"time"
	"libs/chanrpc"
	"sync"
	"libs/log"
	"crypto/tls"
	"net"
	"context"
)

type Client struct {
	AutoReconnect bool
	ConnectInterval time.Duration

	Network string
	Addr string
	DialTimeOut time.Duration
	ConnectNum int
	Protocol
	MsgParser *MsgParser
	SendChanSize int

	ChanServer *chanrpc.Server
	ConnectKey string
	ClosedKey string
	RecvKey string
	UseTLS bool
	Data interface{}

	sessions map[*Session]struct{}
	lock sync.Mutex
	wg sync.WaitGroup
	closeFlag bool
	ctx context.Context
	ctx_cf context.CancelFunc
}

func (c *Client) init() {
	if c.ConnectNum == 0 {
		c.ConnectNum = 1
	}

	if c.SendChanSize == 0 {
		c.SendChanSize = 100
	}

	if c.ConnectInterval == 0 {
		c.ConnectInterval = time.Second * 3
	}

	if c.DialTimeOut == 0 {
		c.DialTimeOut = time.Second * 10
	}

	c.sessions = make(map[*Session]struct{})

	c.ctx, c.ctx_cf = context.WithCancel(context.Background())
}


func (c *Client) Connect() bool {
	c.init()

	c.wg.Add(c.ConnectNum)
	for i := 0;i < c.ConnectNum;i++ {
		go c.connect()
	}

	return true
}

var g_id int64

func (c *Client) connect() {
	defer func(){
		c.wg.Done()
	}()
__RECONNECT:
	var session *Session
	var err error
	var conn net.Conn
	for {
		if c.UseTLS {
			conn, err = tls.Dial(c.Network, c.Addr, &tls.Config{InsecureSkipVerify:true})
			if err == nil {
				session = newSession(conn, c.MsgParser, c.Protocol, c.SendChanSize)
				session.data = c.Data
			}

			if err == nil || c.closeFlag {
				break
			}

		} else {
			conn, err = net.DialTimeout(c.Network, c.Addr, c.DialTimeOut)
			if err == nil {
				session = newSession(conn, c.MsgParser, c.Protocol, c.SendChanSize)
				session.data = c.Data
			}

			if err == nil || c.closeFlag {
				break
			}
		}

		log.Error("connect to %s error:%v", c.Addr, err)
		tc := time.NewTicker(c.ConnectInterval)
		select {
		case <- tc.C:
			tc.Stop()
		case <-c.ctx.Done():
			tc.Stop()
			return
		}
	}

	if session == nil {
		return
	}

	c.lock.Lock()
	if c.closeFlag {
		c.lock.Unlock()
		session.Close()
		return
	}
	c.sessions[session] = struct{}{}
	c.lock.Unlock()

	c.ChanServer.Go(c.ConnectKey, session)
	for {
		msg, err := session.Receive()
		if err != nil {
			log.Debug("session id:%d, addr:%s Receive error:%v", session.Id(), session.RemoteAddr().String(), err)
			break
		}

		c.ChanServer.Go(c.RecvKey, session, msg)
	}
	c.ChanServer.Go(c.ClosedKey, session)

	c.lock.Lock()
	delete(c.sessions, session)
	if c.closeFlag {
		c.lock.Unlock()
		return
	}
	c.lock.Unlock()

	if c.AutoReconnect {
		goto __RECONNECT
	}
}

func (c *Client) Stop() bool {
	c.lock.Lock()
	c.closeFlag = true
	c.ctx_cf()
	for session, _ := range c.sessions {
		session.Close()
	}
	c.lock.Unlock()

	c.wg.Wait()
	return true
}