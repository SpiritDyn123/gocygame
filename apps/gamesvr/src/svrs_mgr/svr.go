package svrs_mgr

import (
	"github.com/SpiritDyn123/gocygame/apps/common/global"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
)

type svrInfo struct {
	session_ global.ILogicSession
	svr_info_ *ProtoMsg.PbSvrBaseInfo
}