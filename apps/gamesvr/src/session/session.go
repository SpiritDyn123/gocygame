package session

import (
	"time"
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"github.com/SpiritDyn123/gocygame/apps/clustersvr/src/global"
	"github.com/SpiritDyn123/gocygame/apps/clustersvr/src/etc"
	"github.com/SpiritDyn123/gocygame/libs/timer"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
)

type ClientSession struct {
	*tcp.Session
	last_check_time_ time.Time
	wtId_ uint64	//心跳定时器 id
	closed_ bool

	svr_info_ *ProtoMsg.PbSvrBaseInfo
}

func CreateSession(session *tcp.Session) (cs *ClientSession) {
	cs = &ClientSession{
		Session: session,
		last_check_time_: time.Now(),
	}
	return
}

func (csession *ClientSession)OnCreate()  {
	tmp_id, err := global.ClusterSvrGlobal.GetWheelTimer().SetTimer(
		uint32(etc.Cluster_Config.System_.Svr_ttl_) * uint32(time.Second / common.Default_Svr_Logic_time),
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


func (csession *ClientSession)OnRecv(data interface{})  {
	//处理内容
	now := time.Now()
	csession.last_check_time_ = now
	msgs := data.([]interface{})
	msg_head := msgs[0].(*common.ProtocolInnerHead)
	if msg_head.GetMsgId() == uint32(ProtoMsg.EmMsgId_MSG_HEART_BEAT) {
		//心跳回复
		csession.Send(msg_head)
		return
	}

	var msg_body []byte
	if len(msgs) > 0 {
		msg_body = msgs[1].([]byte)
	}

	//派发消息
	global.ClusterSvrGlobal.GetMsgDispatcher().Dispatch(csession, msg_head, msg_body)
}

//关闭连接
func (csession *ClientSession)OnClose()  {
	csession.closed_ = true
	log.Debug("session:%+v on closed", csession)
	global.ClusterSvrGlobal.GetWheelTimer().DelTimer(csession.wtId_)
	global.ClusterSvrGlobal.GetSvrsMgr().RemoveSvr(csession, csession.svr_info_)
	csession.Session = nil
}

//心跳检测
func (csession *ClientSession)OnHeartBeat(args ...interface{})  {
	now := time.Now()
	if now.Sub(csession.last_check_time_) > time.Duration(etc.Cluster_Config.System_.Svr_timeout_) * time.Second {
		log.Release("ClientSession:%v onHBTimer timeout", csession)
		csession.Close()
	}
}

func (csession *ClientSession)SetSvrInfo(svr_info *ProtoMsg.PbSvrBaseInfo)  {
	csession.svr_info_ = svr_info
}


func (csession *ClientSession) String() string {
	svr_info := "nil"
	if csession.svr_info_ != nil {
		svr_info = fmt.Sprintf("%v", csession.svr_info_)
	}

	return fmt.Sprintf("{svr_info:%s,last_check_time_:%v}", svr_info, csession.last_check_time_.Format("2006-01-02 15:04:05"))
}