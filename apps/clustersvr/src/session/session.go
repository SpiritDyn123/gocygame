package session

import (
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/clustersvr/src/global"
	common_global "github.com/SpiritDyn123/gocygame/apps/common/global"
	"github.com/SpiritDyn123/gocygame/apps/common/net/session"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
)

type ClusterClientSession struct {
	*session.ClientSession

	svr_info_ *ProtoMsg.PbSvrBaseInfo
}

func CreateSession(tcp_session *tcp.Session, svr_global common_global.IServerGlobal, config_info *ProtoMsg.PbSvrBaseInfo) (cs common_global.ILogicSession) {
	cs = &ClusterClientSession{
		ClientSession: session.CreateClientSession(tcp_session, svr_global, config_info).(*session.ClientSession),
	}
	return
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