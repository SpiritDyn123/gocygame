package session

import (
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/apps/clustersvr/src/global"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/apps/common/net/session"
)

type ClusterClientSession struct {
	*session.ClientSession

	svr_info_ *ProtoMsg.PbSvrBaseInfo
}

func CreateSession(tcp_session *tcp.Session, config_info *ProtoMsg.PbSvrBaseInfo) (cs *ClusterClientSession) {
	cs = &ClusterClientSession{
		ClientSession: session.CreateClientSession(tcp_session, global.ClusterSvrGlobal.GetWheelTimer(), config_info),
	}
	return
}

func (csession *ClusterClientSession)OnRecv(data interface{})  {
	_, is_hb := csession.ClientSession.OnRecv(data)
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
	global.ClusterSvrGlobal.GetMsgDispatcher().Dispatch(csession, msg_head, msg_body)
}

//关闭连接
func (csession *ClusterClientSession) OnClose()  {
	global.ClusterSvrGlobal.GetSvrsMgr().RemoveSvr(csession, csession.svr_info_)

	csession.ClientSession.OnClose()
	csession.ClientSession = nil
}

func (csession *ClusterClientSession)SetSvrInfo(svr_info *ProtoMsg.PbSvrBaseInfo)  {
	csession.svr_info_ = svr_info
}

func (csession *ClusterClientSession) String() string {
	svr_info := "nil"
	if csession.svr_info_ != nil {
		svr_info = fmt.Sprintf("%+v", csession.svr_info_)
	}

	return fmt.Sprintf("ClusterClientSession:{id:%d, info:{%+v}, svr_info:%s,last_check_time_:%v}", csession.Id(),
		csession.Config_info_, svr_info,
		csession.Last_check_time_.Format("2006-01-02 15:04:05"))
}