package tcp

import (
	"github.com/SpiritDyn123/gocygame/libs/chanrpc"
	"github.com/funny/link"
	"github.com/SpiritDyn123/gocygame/libs/log"
)

type Server struct {
	chanServer *chanrpc.Server
	acceptKey string
	recvKey string
	closeKey string
	ser *link.Server
	protocol link.Protocol
}

func (ser *Server) Start() bool {
	//开始accept
	go ser.ser.Serve()

	return true
}

func (ser *Server) Stop() bool {
	ser.ser.Stop()
	return true
}


func (ser *Server) onNewSession(session *link.Session) {
	defer func() {
		session.Close()
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

func CreateServer(network string, addr string, protocol link.Protocol, sendChanSize int, chanServer *chanrpc.Server, acceptKey, recvKey, closeKey string) (ser *Server, err error) {
	ser = &Server{
		chanServer:chanServer,
		acceptKey:acceptKey,
		recvKey:recvKey,
		closeKey:closeKey,
		protocol:protocol,
	}

	ser.ser, err = link.Listen(network, addr, ser.protocol, sendChanSize, link.HandlerFunc(ser.onNewSession))
	if err != nil {
		return
	}

	return
}
