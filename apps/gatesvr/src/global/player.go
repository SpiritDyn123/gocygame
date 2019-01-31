package global

import (
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
)

type IPlayerMgr interface {
	Start() (err error)
	Stop()
	OnAccept(tcp_session *tcp.Session)
	OnRecv(tcp_session *tcp.Session, data interface{})
	OnClose(tcp_session *tcp.Session)
}