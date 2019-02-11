package player

import (
	common_global "github.com/SpiritDyn123/gocygame/apps/common/global"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/apps/gatesvr/src/global"
	"github.com/SpiritDyn123/gocygame/libs/log"
)

type player struct {
	session_ 		common_global.ILogicSession
	game_svr_id_  	int32
	uid_ 			uint64
	b_register_  	bool //首次注册

	login_info_ 	*ProtoMsg.PbCsPlayerLoginReqMsg
	svr_type_in_   map[ProtoMsg.EmSvrType]int32

}

func (p *player) OnRecv(head common.IMsgHead, msg interface{}) {
	svr_mgr := global.GateSvrGlobal.GetSvrsMgr()
	svr_type := svr_mgr.GetSvrTypeCSId(head.GetMsgId())
	if svr_type == ProtoMsg.EmSvrType_ST_Invalid {
		log.Error("player::player GetSvrTypeCSId:%d invalid svr type", head.GetMsgId())
		p.session_.Close()
		return
	}

	if _, ok := p.svr_type_in_[svr_type]; ok {
		svr_mgr.SendBySvrId(svr_type, p.svr_type_in_[svr_type], head, msg)
	} else {
		svr_mgr.SendByCSId(p.uid_, head, msg) //功能性的无状态服务
	}
}

func (p *player)  OnRecvSvr(head common.IMsgHead, msg interface{}) {
	mhead := head.(*common.ProtocolInnerHead)
	cli_mhead := &common.ProtocolClientHead{
		Msg_id_: mhead.Msg_id_,
		Seq_: mhead.Seq_,
		Uid_: p.uid_,
	}
	p.session_.Send(cli_mhead, msg)
}

func (p *player) login() {

}