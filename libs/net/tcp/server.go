package tcp

import (
	"crypto/tls"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"github.com/SpiritDyn123/gocygame/libs/chanrpc"
	"net"
	"time"
	"sync"
)

type INetServer interface {
	Start() bool
	Stop() bool
}

type tcpServer struct {
	chanServer *chanrpc.Server
	acceptKey  string
	recvKey    string
	closeKey   string
	sendChanSize int
	listener net.Listener
	sessions sync.Map

	msgParser IMsgParser
	protocol Protocol

	stopOnce sync.Once
}

func (ser *tcpServer) Start() bool {
	//开始accept
	go ser.serve()

	return true
}

func (ser *tcpServer) serve() {
	var tempDelay time.Duration
	for {
		conn, err := ser.listener.Accept()
		if err != nil {
			if ne, ok := err.(net.Error); ok && ne.Temporary() {
				if tempDelay == 0 {
					tempDelay = 5 * time.Millisecond
				} else {
					tempDelay *= 2
				}
				if max := 1 * time.Second; tempDelay > max {
					tempDelay = max
				}
				time.Sleep(tempDelay)
				continue
			}
			log.Error("tcp server Accept error:%v", err)
			return
		}

		go ser.handleNewConn(conn)
	}
}

func (ser *tcpServer) Stop() bool {
	ser.listener.Close()

	ser.stopOnce.Do(func(){
		ser.sessions.Range(func(id interface{}, session interface{}) bool {
			session.(*Session).Close()
			return true
		})
	})

	return true
}

func (ser *tcpServer) handleNewConn(conn net.Conn) {
	session := newSession(conn, Session_type_tcp, ser.msgParser, ser.protocol, ser.sendChanSize)
	ser.sessions.Store(session.Id(), session)

	defer func() {
		session.Close()
		ser.sessions.Delete(session.Id())
	}()

	ser.chanServer.Go(ser.acceptKey, session)
	for {
		idata, err := session.Receive()
		if err != nil {
			log.Debug("socket %v read error:%v", session, err)
			ser.chanServer.Go(ser.closeKey, session)
			return
		}

		ser.chanServer.Go(ser.recvKey, session, idata)
	}
}

func CreateTcpServer(network, addr string, protocol Protocol, msgParser IMsgParser,
	sendChanSize int, chanServer *chanrpc.Server, acceptKey, recvKey, closeKey string) (INetServer, error) {
	ser := &tcpServer{
		chanServer: chanServer,
		acceptKey:  acceptKey,
		recvKey:    recvKey,
		closeKey:   closeKey,
		msgParser: msgParser,
		protocol: protocol,
		sendChanSize: sendChanSize,
	}

	if ser.msgParser == nil {
		ser.msgParser = NewMsgParser()
	}

	var err error
	ser.listener, err = net.Listen(network, addr)
	return ser, err
}

func CreateTLSServer(network string, addr string, certFile string, keyFile string, protocol Protocol, msgParser IMsgParser,
	sendChanSize int, chanServer *chanrpc.Server, acceptKey, recvKey, closeKey string) (INetServer, error) {
	cert, err := tls.LoadX509KeyPair(certFile, keyFile)
	if err != nil {
		return nil, err
	}

	tlsConf := &tls.Config{
		Certificates: []tls.Certificate{cert},
	}

	ser := &tcpServer{
		chanServer: chanServer,
		acceptKey:  acceptKey,
		recvKey:    recvKey,
		closeKey:   closeKey,
		msgParser: msgParser,
		protocol: protocol,
		sendChanSize: sendChanSize,
	}

	ser.listener, err = tls.Listen(network, addr, tlsConf)
	return ser, err
}
