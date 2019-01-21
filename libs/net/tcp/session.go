package tcp

import (
	"net"
	"sync/atomic"
	"libs/log"
	"sync"
	"errors"
)

var (
	Error_Session_Closed = errors.New("session closed")
	Error_Session_Send_Chan_Full = errors.New("session send chan full")
)

type SessionType int
const (
	Session_type_tcp SessionType = 1 +iota //tcp基本链接
	Session_type_ws
)

type netConn interface {
	Close() error
	LocalAddr() net.Addr
	RemoteAddr() net.Addr
}

var global_sessionId uint64

type Session struct {
	msgParser IMsgParser
	codec Codec
	sessionId uint64
	conn netConn
	sessionType SessionType

	sendChan chan interface{}
	closeChan chan struct{}
	closeFlag bool
	lock sync.RWMutex

	data interface{}
}

func (session *Session) Id() uint64 {
	return session.sessionId
}

func (session *Session) Send(msg ...interface{}) error {
	session.lock.RLock()
	if session.closeFlag {
		session.lock.RUnlock()
		return Error_Session_Closed
	}

	select {
	case session.sendChan <- msg:
	default:
		session.lock.RUnlock()
		log.Error("session send chan full cap:%d", cap(session.sendChan))
		session.Close()
		return Error_Session_Send_Chan_Full
	}
	session.lock.RUnlock()

	return nil
}

func (session *Session) Receive() (interface{}, error) {
	data, err := session.msgParser.Read(session.conn)
	if err != nil {
		return nil, err
	}

	return session.codec.Unmarshal(data)
}


func (session *Session) Close() {
	session.lock.Lock()
	defer session.lock.Unlock()
	if session.closeFlag {
		return
	}

	session.closeFlag = true
	if session.conn != nil {

	}
	session.conn.Close()
	close(session.closeChan)
	close(session.sendChan)

	//获取未读取到的数据
	if csc, ok := session.codec.(CloseSendChan); ok {
		csc.CloseEnd(session.sendChan)
	}
}

func (session *Session) sendLoop() {
	for {
		select {
		case <- session.closeChan:
			goto __END
		case msg, ok := <- session.sendChan:
			if !ok {
				goto __END
			}
			data, err := session.codec.Marshal(msg)
			if err != nil {
				log.Error("session sendLoop codec.Marshal err:%v", err)
				goto __END
			}

			err = session.msgParser.Write(session.conn, data)
			if err != nil {
				log.Error("session sendLoop msgParser.Write err:%v", err)
				goto __END
			}
		}
	}

__END:
	session.Close()
}

func (session *Session) RemoteAddr() net.Addr {
	return session.conn.RemoteAddr()
}

func (session *Session) Data() interface{} {
	return session.data
}

func (session *Session) SessionType() SessionType {
	return session.sessionType
}

func newSession(conn netConn, session_type SessionType, msgParser IMsgParser, protocol Protocol, sendChanSize int) *Session {
	sessionId := atomic.AddUint64(&global_sessionId, 1)
	session := &Session{
		conn: conn,
		sessionId: sessionId,
		sessionType: session_type,
		msgParser: msgParser,
		codec: protocol.NewCodec(),
		sendChan: make(chan interface{}, sendChanSize),
		closeChan: make(chan struct{}),
	}

	//开启写协程
	go session.sendLoop()

	return session
}
