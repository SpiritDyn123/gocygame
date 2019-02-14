package global

import (
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
)

const (
	Session_attribute_player_id = "Session_attribute_player_id"
)

type IPlayer interface {
	OnRecv(head common.IMsgHead, msg interface{})
	OnRecvSvr(head common.IMsgHead, msg interface{})
}

type IPlayerMgr interface {
	Start() (err error)
	Stop()
	OnAccept(tcp_session *tcp.Session)
	OnRecv(tcp_session *tcp.Session, data interface{})
	OnClose(tcp_session *tcp.Session)
	GetPlayerById(uint64) IPlayer
	BroadClientMsg(head common.IMsgHead, msg interface{})
	OnSvrClosed(cfg_svr_info *ProtoMsg.PbSvrBaseInfo)
}