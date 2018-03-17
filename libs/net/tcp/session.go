package tcp

import (
	"net"
	"sync/atomic"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"sync"
	"github.com/pkg/errors"
)

var (
	Error_Session_Closed = errors.New("session closed")
	Error_Session_Send_Chan_Full = errors.New("session send chan full")
)

var global_sessionId uint64

type Session struct {
	msgParser *MsgParser
	codec Codec
	sessionId uint64
	conn net.Conn

	sendChan chan interface{}
	closeChan chan struct{}
	closeFlag bool
	lock sync.RWMutex
}

func (session *Session) Id() uint64 {
	return session.sessionId
}

func (session *Session) Send(msg interface{}) error {
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
		case msg := <- session.sendChan:
			data, err := session.codec.Marshal(msg)
			if err != nil {
				goto __END
			}

			err = session.msgParser.Write(session.conn, data)
			if err != nil {
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

func newSession(conn net.Conn, msgParser *MsgParser, codec Codec, sendChanSize int) *Session {
	sessionId := atomic.AddUint64(&global_sessionId, 1)
	session := &Session{
		conn: conn,
		sessionId: sessionId,
		msgParser: msgParser,
		codec: codec,
		sendChan: make(chan interface{}, sendChanSize),
		closeChan: make(chan struct{}),
	}

	//开启写协程
	go session.sendLoop()

	return session
}