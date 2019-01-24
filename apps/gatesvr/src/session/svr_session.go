package session

import (
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/gatesvr/src/etc"
	"github.com/SpiritDyn123/gocygame/apps/gatesvr/src/global"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/apps/common/net/session"
)


var Publish_Servers = []ProtoMsg.EmSvrType{
	ProtoMsg.EmSvrType_Gs,
}

func CreateSvrSession(tcp_session *tcp.Session, config_info *ProtoMsg.PbSvrBaseInfo) *GateSvrSession {
	return &GateSvrSession{
		SvrSession: session.CreateSvrSession(tcp_session, global.GateSvrGlobal.GetWheelTimer(), config_info),
	}
}

type GateSvrSession struct {
	*session.SvrSession
}

func (ssession *GateSvrSession)OnCreate()  {
	ssession.SvrSession.OnCreate()

	//发送注册和订阅消息
	head := &common.ProtocolInnerHead{
		Msg_id_: uint32(ProtoMsg.EmMsgId_SVR_MSG_REGISTER_CLUSTER),
	}

	reg_msg := &ProtoMsg.PbSvrRegisterClusterReqMsg{
		SvrInfo: &ProtoMsg.PbSvrBaseInfo{
			GroupId: int32(etc.Gate_Config.System_.Svr_group_id_),
			SvrId: int32(etc.Gate_Config.System_.Svr_group_id_),
			SvrType: ProtoMsg.EmSvrType_Gate,
			Addr: etc.Gate_Config.System_.Svr_addr_,
			Ttl: int32(etc.Gate_Config.System_.Svr_ttl_), //用系统监听的ttl
			Timeout: int32(etc.Gate_Config.System_.Svr_timeout_),
		},
		SvrTypes: Publish_Servers,
	}

	ssession.Send(head, reg_msg)
}

func (ssession *GateSvrSession) OnRecv(data interface{})  {
	_, is_hb := ssession.SvrSession.OnRecv(data)
	if is_hb {
		return
	}

	//处理内容
	msgs := data.([]interface{})
	msg_head := msgs[0].(*common.ProtocolInnerHead)
	var msg_body []byte
	if len(msgs) > 0 {
		msg_body = msgs[1].([]byte)
	}

	//派发消息
	global.GateSvrGlobal.GetMsgDispatcher().Dispatch(ssession, msg_head, msg_body)
}

func (ssession *GateSvrSession) String() string {
	return fmt.Sprintf("GateSvrSession:{id:%d, info:{%+v}, last_check_time_:%v}", ssession.Id(),
		ssession.Config_info_, ssession.Last_check_time_.Format("2006-01-02 15:04:05"))
}

