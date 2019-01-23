package session

import (
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/gatesvr/src/etc"
	"github.com/SpiritDyn123/gocygame/apps/gatesvr/src/global"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/libs/timer"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"time"
)


var Publish_Servers = []ProtoMsg.EmSvrType{
	ProtoMsg.EmSvrType_Gs,
}

type SvrSession struct {
	*tcp.Session

	svr_info_ *ProtoMsg.PbSvrBaseInfo

	last_check_time_ time.Time

	wtId_ uint64
}

func (ssession *SvrSession)OnCreate()  {
	tmp_id, err := global.GateSvrGlobal.GetWheelTimer().SetTimer(
		uint32(ssession.svr_info_.Ttl) * uint32(time.Second / common.Default_Svr_Logic_time),
		true, timer.TimerHandlerFunc(ssession.OnHeartBeat), 0)

	ssession.last_check_time_ = time.Now()

	if err != nil {
		ssession.Close()
		log.Error("SvrSession register heatbeat timer err:%v", err)
		return
	}
	ssession.wtId_ = tmp_id

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
			Ttl: int32(ssession.svr_info_.Ttl),
			Timeout: int32(ssession.svr_info_.Timeout),
		},
		SvrTypes: Publish_Servers,
	}

	ssession.Send(head, reg_msg)
}

func (ssession *SvrSession) Send(msg ...interface{}) error {
	if ssession.Session == nil {
		return fmt.Errorf("send in closed client session")
	}
	return ssession.Session.Send(msg...)
}


func (ssession *SvrSession)OnRecv(data interface{})  {
	//处理内容
	now := time.Now()
	ssession.last_check_time_ = now
	msgs := data.([]interface{})
	msg_head := msgs[0].(*common.ProtocolInnerHead)
	if msg_head.GetMsgId() == uint32(ProtoMsg.EmMsgId_MSG_HEART_BEAT) {
		return
	}

	var msg_body []byte
	if len(msgs) > 0 {
		msg_body = msgs[1].([]byte)
	}

	//派发消息
	global.GateSvrGlobal.GetMsgDispatcher().Dispatch(ssession, msg_head, msg_body)
}

//关闭连接
func (ssession *SvrSession)OnClose()  {
	log.Debug("session:%+v on closed", ssession)
	global.GateSvrGlobal.GetWheelTimer().DelTimer(ssession.wtId_)
	ssession.Session = nil
}

//心跳检测
func (ssession *SvrSession)OnHeartBeat(args ...interface{})  {
	now := time.Now()
	if now.Sub(ssession.last_check_time_) > time.Duration(ssession.svr_info_.Timeout) * time.Second {
		log.Release("ClientSession:%v onHBTimer timeout", ssession)
		ssession.Close()
	}

	//发送心跳
	ssession.Send(&common.ProtocolInnerHead{
		Msg_id_: uint32(ProtoMsg.EmMsgId_MSG_HEART_BEAT),
	})
}

func (ssession *SvrSession) String() string {
	return fmt.Sprintf("{svr_info:%s, last_check_time_:%v}",
		fmt.Sprintf("%v", ssession.svr_info_), ssession.last_check_time_.Format("2006-01-02 15:04:05"))
}

func CreateSvrSession(tcp_session *tcp.Session, svr_info *ProtoMsg.PbSvrBaseInfo) *SvrSession {
	return &SvrSession{
		Session: tcp_session,
		svr_info_: svr_info,
	}
}
