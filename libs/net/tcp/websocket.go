package tcp

import (
	"crypto/tls"
	"net"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"time"
	"net/http"
	"github.com/gorilla/websocket"
	"github.com/SpiritDyn123/gocygame/libs/chanrpc"
	"errors"
)

type wsMsgParse struct {
	max_send int64
	max_recv int64
}

func(wp *wsMsgParse) Read(r interface{}) ([]byte, error) {
	ws_conn := r.(*websocket.Conn)

	//长度限制在ws conn 的	SetReadLimit(ser.max_recv)
	_, data, err := ws_conn.ReadMessage()
	return data, err
}

func(wp *wsMsgParse) Write(w interface{}, args ...[]byte) error {
	ws_conn := w.(*websocket.Conn)

	var msgLen int64
	for i := 0; i < len(args); i++ {
		msgLen += int64(len(args[i]))
	}

	// check len
	if msgLen > wp.max_send {
		return errors.New("message too long")
	} else if msgLen < 1 {
		return errors.New("message too short")
	}

	if len(args) == 1 {
		ws_conn.WriteMessage(websocket.BinaryMessage, args[0])
		return nil
	}

	msg := make([]byte, msgLen)
	l := 0
	for i := 0; i < len(args); i++ {
		copy(msg[l:], args[i])
		l += len(args[i])
	}

	err := ws_conn.WriteMessage(websocket.BinaryMessage, msg)
	return err
}

type wsServer struct {
	ser_base tcpServer
	max_recv int64
	max_send int64
	read_time_out time.Duration
	write_time_out time.Duration

	upgrader  websocket.Upgrader
}

func (ser *wsServer) Start() bool {
	ws_ser := &http.Server{
		Addr:           ser.ser_base.listener.Addr().String(),
		Handler:        ser,
		MaxHeaderBytes: 1024,
		ReadTimeout:    ser.read_time_out,
		WriteTimeout:   ser.write_time_out,
	}

	go func() {
		err := ws_ser.Serve(ser.ser_base.listener)
		if err != nil {
			panic(err)
		}
	}()

	return true
}


func (ser *wsServer) Stop() bool {
	ser.ser_base.listener.Close()

	ser.ser_base.stopOnce.Do(func(){
		ser.ser_base.sessions.Range(func(id interface{}, session interface{}) bool {
			session.(*Session).Close()
			return true
		})
	})

	return true
}

func (ser *wsServer) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	if r.Method != "GET" {
		http.Error(w, "wssocket Method not allowed", 405)
		return
	}
	conn, err := ser.upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Debug("wssocket upgrade error: %v", err)
		return
	}

	conn.SetReadLimit(ser.max_recv)

	session := newSession(conn, Session_type_ws, ser.ser_base.msgParser, ser.ser_base.protocol, ser.ser_base.sendChanSize)
	ser.ser_base.sessions.Store(session.Id(), session)

	defer func() {
		session.Close()
		ser.ser_base.sessions.Delete(session.Id())
	}()

	ser.ser_base.chanServer.Go(ser.ser_base.acceptKey, session)
	for {
		idata, err := session.Receive()
		if err != nil {
			log.Debug("ws socket %v read error:%v", session, err)
			ser.ser_base.chanServer.Go(ser.ser_base.closeKey, session)
			return
		}

		ser.ser_base.chanServer.Go(ser.ser_base.recvKey, session, idata)
	}
}

func CreateWSServer(network string, addr string, certFile string, keyFile string, protocol Protocol,
	max_recv int64, max_send int64, read_time_out time.Duration, write_time_out time.Duration,
	sendChanSize int, chanServer *chanrpc.Server, acceptKey, recvKey, closeKey string) (INetServer, error) {

	ln, err := net.Listen(network, addr)
	if err != nil {
		return nil, err
	}

	ser := &wsServer{
		ser_base: tcpServer{
			chanServer: chanServer,
			acceptKey:  acceptKey,
			recvKey:    recvKey,
			closeKey:   closeKey,
			protocol: protocol,
			sendChanSize: sendChanSize,
			msgParser: &wsMsgParse{
				max_send: max_send,
				max_recv: max_recv,
			},
		},
		max_recv: max_recv,
		max_send: max_send,
		read_time_out: read_time_out,
		write_time_out: write_time_out,

		upgrader: websocket.Upgrader{
			HandshakeTimeout: 3 * time.Second,
			CheckOrigin:      func(_ *http.Request) bool { return true },
		},
	}

	//判断是否是WSS
	if certFile != "" || keyFile != "" {
		cert, err := tls.LoadX509KeyPair(certFile, keyFile)
		if err != nil {
			return nil, err
		}

		tlsConf := &tls.Config{
			Certificates: []tls.Certificate{cert},
		}

		ln = tls.NewListener(ln, tlsConf)
	}
	ser.ser_base.listener = ln

	return ser, nil
}