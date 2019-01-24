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
	cs = &ClientSession{
		BaseSession: BaseSession{
			Session: tcp_session,
			Svr_global_: svr_global,
			Config_info_: config_info,
		},
	}
	return
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
}

func (csession *ClientSession) Send(msg ...interface{}) error {
	if csession.Session == nil {
		return fmt.Errorf("send in closed client session")
	}
	return csession.Session.Send(msg...)
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
	csession.Svr_global_.GetMsgDispatcher().Dispatch(csession, msg_head, msg_body)

	return
}

//关闭连接
func (csession *ClientSession) OnClose()  {
	csession.closed_ = true
	//log.Debug("session:%+v on closed", csession)
	csession.Svr_global_.GetWheelTimer().DelTimer(csession.wtId_)
	csession.Session = nil
}

//心跳检测
func (csession *ClientSession)OnHeartBeat(args ...interface{})  {
	now := time.Now()
	if now.Sub(csession.Last_check_time_) > time.Duration(csession.Config_info_.Timeout) * time.Second {
		log.Release("ClientSession:%v onHBTimer timeout", csession)
		csession.Close()
	}
}

func (csession *ClientSession) String() string {
	return fmt.Sprintf("ClientSession:{id:%d, info:{%+v}, closed:%v, last_check_time_:%v}", csession.Id(),
		csession.Config_info_, csession.closed_,
		csession.Last_check_time_.Format("2006-01-02 15:04:05"))
}