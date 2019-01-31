package session

import (
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/apps/common/global"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/libs/timer"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"time"
)

func CreateSvrSession(tcp_session *tcp.Session, svr_global global.IServerGlobal, config_info *ProtoMsg.PbSvrBaseInfo) (ss global.ILogicSession) {
	ss = &SvrSession{
		BaseSession: BaseSession{
			Session: tcp_session,
			Svr_global_: svr_global,
			Config_info_: config_info,
			M_event_cbs_: make(map[SessionEvent]SessionEventCallBack),

			m_attribute_: make(map[string]interface{}),
		},
	}
	return
}

type SvrSession struct {
	BaseSession

	wtId_ uint64
}

func (ssession *SvrSession)OnCreate()  {
	ssession.Last_check_time_ = time.Now()

	tmp_id, err := ssession.Svr_global_.GetWheelTimer().SetTimer(
		uint32(ssession.Config_info_.Ttl) * uint32(time.Second / common.Default_Svr_Logic_time),
		true, timer.TimerHandlerFunc(ssession.OnHeartBeat), 0)
	if err != nil {
		ssession.Close()
		log.Error("SvrSession register heatbeat timer err:%v", err)
		return
	}
	ssession.wtId_ = tmp_id

	//连接回调
	if cb, ok := ssession.M_event_cbs_[SessionEvent_Accept]; ok {
		cb(ssession)
	}
}

func (ssession *SvrSession)OnRecv(data interface{}) (now time.Time, is_hb bool)   {
	//处理内容
	now = time.Now()
	ssession.Last_check_time_ = now
	msgs := data.([]interface{})
	msg_head := msgs[0].(common.IMsgHead)
	if msg_head.GetMsgId() == uint32(ProtoMsg.EmSSMsgId_SVR_MSG_HEART_BEAT) {
		is_hb = true
		return
	}

	//处理内容
	var msg_body []byte
	if len(msgs) > 0 {
		msg_body = msgs[1].([]byte)
	}

	//派发消息
	err := ssession.Svr_global_.GetMsgDispatcher().Dispatch(ssession, msg_head, msg_body)
	if err != nil {
		log.Error("ClientSession::OnRecv err:%v", err)
	}

	//连接回调
	if cb, ok := ssession.M_event_cbs_[SessionEvent_Recv]; ok {
		cb(ssession)
	}

	return
}

//关闭连接
func (ssession *SvrSession)OnClose()  {
	//log.Debug("session:%+v on closed", ssession)
	ssession.Svr_global_.GetWheelTimer().DelTimer(ssession.wtId_)
	//连接回调
	if cb, ok := ssession.M_event_cbs_[SessionEvent_Close]; ok {
		cb(ssession)
	}

	ssession.Session = nil
	ssession.Svr_global_ = nil
	ssession.Config_info_ = nil
	ssession.m_attribute_ = nil
	ssession.M_event_cbs_ = nil
}

//心跳检测
func (ssession *SvrSession)OnHeartBeat(args ...interface{})  {
	now := time.Now()
	if now.Sub(ssession.Last_check_time_) > time.Duration(ssession.Config_info_.Timeout) * time.Second {
		log.Release("%v onHBTimer timeout second:%d", ssession, ssession.Config_info_.Timeout)
		ssession.Close()
	} else {
		//发送心跳
		ssession.Send(&common.ProtocolInnerHead{
			Msg_id_: uint32(ProtoMsg.EmSSMsgId_SVR_MSG_HEART_BEAT),
		})
	}
}

func (ssession *SvrSession) String() string {
	return fmt.Sprintf("SvrSession:{id:%d, info:{%+v}, closed:%v, last_check_time_:%v}", ssession.Id(),
		ssession.Config_info_, ssession.closed_,
		ssession.Last_check_time_.Format("2006-01-02 15:04:05"))
}
