package login

import (
	"github.com/SpiritDyn123/gocygame/apps/loginsvr/src/global"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/apps/common"
	common_global"github.com/SpiritDyn123/gocygame/apps/common/global"
)

var LoginMgr global.ILoginMgr
func init() {
	LoginMgr = &loginMgr{}
}

type loginMgr struct {

}

func (mgr *loginMgr) Start() (err error) {
	global.LoginSvrGlobal.GetMsgDispatcher().Register(uint32(ProtoMsg.EmCSMsgId_CS_MSG_PLAYER_LOGIN),
		&ProtoMsg.PbCsPlayerLoginReqMsg{}, mgr.onRecvPlayerLogin)

	return
}

func (mgr *loginMgr) Stop() {

}

//用户登陆
func (mgr *loginMgr) onRecvPlayerLogin(sink interface{}, head common.IMsgHead, msg interface{}) {
	resp_msg := &ProtoMsg.PbCsPlayerLoginResMsg{
		Ret: &ProtoMsg.Ret{
			ErrCode: 0,
		},
		Uid: 10000,
	}
	sink.(common_global.ILogicSession).Send(head, resp_msg)
}