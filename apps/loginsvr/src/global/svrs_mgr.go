package global

import (
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/golang/protobuf/proto"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
)

const (
	Session_attribute_key_svr_info = "attribute_svr_info"
)
type ISvrsMgr interface {
	Start() error
	Stop()
	OnAccept(session *tcp.Session)
	OnClose(session *tcp.Session)
	OnRecv(session *tcp.Session, data interface{})
	SendToSvr(svrType ProtoMsg.EmSvrType, svr_id int32, head common.IMsgHead, msg proto.Message)
}
