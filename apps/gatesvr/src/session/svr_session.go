package session

import (
	"fmt"
	common_global "github.com/SpiritDyn123/gocygame/apps/common/global"
	"github.com/SpiritDyn123/gocygame/apps/common/net/session"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
)


func CreateSvrSession(tcp_session *tcp.Session, svr_global common_global.IServerGlobal, config_info *ProtoMsg.PbSvrBaseInfo) common_global.ILogicSession {
	return &GateSvrSession{
		SvrSession: session.CreateSvrSession(tcp_session, svr_global, config_info).(*session.SvrSession),
	}
}

type GateSvrSession struct {
	*session.SvrSession
}

func (ssession *GateSvrSession)OnCreate()  {
	ssession.SvrSession.OnCreate()
}

func (ssession *GateSvrSession) String() string {
	return fmt.Sprintf("GateSvrSession:{id:%d, info:{%+v}, last_check_time_:%v}", ssession.Id(),
		ssession.Config_info_, ssession.Last_check_time_.Format("2006-01-02 15:04:05"))
}

