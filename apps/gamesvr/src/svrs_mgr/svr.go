package svrs_mgr

import (
	"github.com/SpiritDyn123/gocygame/apps/common/global"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/apps/common/net/strategy"
)

type svrInfo struct {
	session_ global.ILogicSession
	svr_info_ *ProtoMsg.PbSvrBaseInfo
}

type svrGroup struct {
	m_svrs_info_ map[int32]*svrInfo
	svr_type_ ProtoMsg.EmSvrType
	selector_ strategy.Selector
}

func(sg *svrGroup) OnCreate() {
	if sg.svr_type_ == ProtoMsg.EmSvrType_DB {

	} else if sg.svr_type_ == ProtoMsg.EmSvrType_Gate {

	}

}

func(sg *svrGroup) Remove() {

}

