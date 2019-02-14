package player

import (
	"github.com/SpiritDyn123/gocygame/apps/gamesvr/src/global"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/apps/common"
	common_global"github.com/SpiritDyn123/gocygame/apps/common/global"
	"libs/log"
	"github.com/SpiritDyn123/gocygame/apps/gamesvr/src/etc"
)

var PlayerMgr global.IPlayerMgr
func init() {
	PlayerMgr = &playerMgr{}
}

type playerMgr struct {

}

func(mgr *playerMgr) Start() (err error) {
	global.GameSvrGlobal.GetMsgDispatcher().Register(uint32(ProtoMsg.EmSSMsgId_SVR_MSG_GAME_LOGIN),
		&ProtoMsg.PbSvrGameLoginReqMsg{}, mgr.onRecvPlayerLogin)

	return
}

func(mgr *playerMgr) Stop() {

}

func (mgr *playerMgr) onRecvPlayerLogin(sink interface{}, head common.IMsgHead, msg interface{}) {
	req_msg := msg.(*ProtoMsg.PbSvrGameLoginReqMsg)
	session := sink.(common_global.ILogicSession)
	if req_msg.Online { //上线
		resp_msg := &ProtoMsg.PbCsPlayerLoginResMsg{
			Ret: &ProtoMsg.Ret{
				ErrCode: 0,
			},
			Uid: req_msg.Uid,
		}
		session.Send(head, resp_msg)

		//通知用户游戏服id
		session.Send(&common.ProtocolInnerHead{
			Msg_id_: uint32(ProtoMsg.EmSSMsgId_SVR_MSG_COMMON_PUSH_PLAYER_SVRID),
		}, &ProtoMsg.PbSvrCommonPushPlayerSvrId{
			Uid: req_msg.Uid,
			SvrType: ProtoMsg.EmSvrType_Gs,
			SvrId: int32(etc.Game_Config.System_.Svr_id_),
			Add: true,
		})
	} else { //下线

	}

	log.Release("playerMgr::onRecvPlayerLogin player:%d online:%v", req_msg.Uid, req_msg.Online)

	//通知world服务器
	world_head := &common.ProtocolInnerHead{
		Msg_id_: uint32(ProtoMsg.EmSSMsgId_SVR_MSG_WORLD_LOGIN),
		Seq_: 0,
		Uid_lst_: []uint64{req_msg.Uid},
	}
	world_req_msg := &ProtoMsg.PbSvrWorldLoginReqMsg{
		Uid: req_msg.Uid,
		Online: req_msg.Online,
	}
	global.GameSvrGlobal.GetSvrsMgr().SendBySvrType(req_msg.Uid, ProtoMsg.EmSvrType_World, world_head, world_req_msg)
}



