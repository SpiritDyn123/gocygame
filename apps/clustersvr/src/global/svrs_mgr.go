package global

import "github.com/SpiritDyn123/gocygame/apps/common/proto"

type ISvrsMgr interface {
	Start() error
	Stop()
	RemoveSvr(session interface{}, svr_info *ProtoMsg.PbSvrBaseInfo)
}