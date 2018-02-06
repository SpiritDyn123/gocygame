package tcp

import (
	"time"
	"github.com/funny/link"
	"github.com/SpiritDyn123/gocygame/libs/chanrpc"
	"sync"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"crypto/tls"
)

type Client struct {
	AutoReconnect bool
	ConnectInterval time.Duration

	Network string
	Addr string
	DialTimeOut time.Duration
	ConnectNum int
	Protocol link.Protocol
	SendChanSize int

	ChanServer *chanrpc.Server
	ConnectKey string
	ClosedKey string
	RecvKey string
	UseTLS bool

	sessions map[*link.Session]struct{}
	lock sync.Mutex
	wg sync.WaitGroup
	closeFlag bool
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

	c.sessions = make(map[*link.Session]struct{})
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
	var session *link.Session
	var err error
	for {
		if c.UseTLS {
			conn, err := tls.Dial(c.Network, c.Addr, &tls.Config{InsecureSkipVerify:true})
			if err == nil {
				codec, err := c.Protocol.NewCodec(conn)
				if err == nil {
					session = link.NewSession(codec, c.SendChanSize)
					break
				} else {
					if c.closeFlag {
						break
					}
				}
			} else {
				if c.closeFlag {
					break
				}
			}
		} else {
			session, err = link.DialTimeout(c.Network, c.Addr, c.DialTimeOut, c.Protocol, c.SendChanSize)
			if err == nil || c.closeFlag {
				break
			}
		}

		log.Error("connect to %s error:%v", c.Addr, err)
		time.Sleep(c.ConnectInterval)
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
			log.Debug("session:%v Receive error:%v", session, err)
			break
		}

		c.ChanServer.Go(c.RecvKey, session, msg)
	}
	c.ChanServer.Go(c.ClosedKey, session)

	c.lock.Lock()
	delete(c.sessions, session)
	c.lock.Unlock()

	if c.AutoReconnect {
		goto __RECONNECT
	}
}

func (c *Client) Stop() bool {
	c.lock.Lock()
	c.closeFlag = true
	for session, _ := range c.sessions {
		session.Close()
	}
	c.lock.Unlock()

	c.wg.Wait()
	return true
}