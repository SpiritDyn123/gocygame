package session

import (
	"github.com/SpiritDyn123/gocygame/apps/common/global"
	"time"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"fmt"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/libs/timer"

)

func CreateClientSession(tcp_session *tcp.Session, svr_global global.IServerGlobal, config_info *ProtoMsg.PbSvrBaseInfo) (cs global.ILogicSession) {
	return &ClientSession{
		BaseSession: BaseSession{
			Session: tcp_session,
			Svr_global_: svr_global,
			Config_info_: config_info,
			M_event_cbs_: make(map[SessionEvent]SessionEventCallBack),

			m_attribute_: make(map[string]interface{}),
		},
	}
}

type ClientSession struct {
	BaseSession
}

func (csession *ClientSession)OnCreate()  {
	csession.Last_check_time_ = time.Now()

	tmp_id, err := csession.Svr_global_.GetWheelTimer().SetTimer(
		uint32(csession.Config_info_.Ttl) * uint32(time.Second / common.Default_Svr_Logic_time),
		true, timer.TimerHandlerFunc(csession.OnHeartBeat), 0)
	if err != nil {
		csession.Close()
		log.Error("ClientSession register heatbeat timer err:%v", err)
		return
	}
	csession.wtId_ = tmp_id

	//连接回调
	if cb, ok := csession.M_event_cbs_[SessionEvent_Accept]; ok {
		cb(csession)
	}
}


func (csession *ClientSession)OnRecv(data interface{}) (now time.Time, is_hb bool)  {
	//处理内容
	now = time.Now()
	csession.Last_check_time_ = now
	msgs := data.([]interface{})
	msg_head := msgs[0].(common.IMsgHead)
	if msg_head.GetMsgId() == uint32(ProtoMsg.EmMsgId_MSG_HEART_BEAT) {
		is_hb = true
		//心跳回复
		csession.Send(msg_head)
		return
	}

	//处理内容
	var msg_body []byte
	if len(msgs) > 0 {
		msg_body = msgs[1].([]byte)
	}

	//派发消息
	err := csession.Svr_global_.GetMsgDispatcher().Dispatch(csession, msg_head, msg_body)
	if err != nil {
		log.Error("ClientSession::OnRecv err:%v", err)
	}

	//连接回调
	if cb, ok := csession.M_event_cbs_[SessionEvent_Recv]; ok {
		cb(csession)
	}

	return
}

//关闭连接
func (csession *ClientSession) OnClose()  {
	csession.closed_ = true
	//log.Debug("session:%+v on closed", csession)
	csession.Svr_global_.GetWheelTimer().DelTimer(csession.wtId_)

	//连接回调
	if cb, ok := csession.M_event_cbs_[SessionEvent_Close]; ok {
		cb(csession)
	}

	csession.Session = nil
	csession.Svr_global_ = nil
	csession.Config_info_ = nil
	csession.m_attribute_ = nil
	csession.M_event_cbs_ = nil
}

//心跳检测
func (csession *ClientSession)OnHeartBeat(args ...interface{})  {
	now := time.Now()
	if now.Sub(csession.Last_check_time_) > time.Duration(csession.Config_info_.Timeout) * time.Second {
		log.Release("%v onHBTimer timeout second:%d", csession, csession.Config_info_.Timeout)
		csession.Close()
	}
}

func (csession *ClientSession) String() string {
	return fmt.Sprintf("ClientSession:{id:%d, info:{%+v}, closed:%v, last_check_time_:%v}", csession.Id(),
		csession.Config_info_, csession.closed_,
		csession.Last_check_time_.Format("2006-01-02 15:04:05"))
}