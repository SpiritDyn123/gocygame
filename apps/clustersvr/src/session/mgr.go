package session

import (
	"github.com/SpiritDyn123/gocygame/apps/clustersvr/src/etc"
	"github.com/SpiritDyn123/gocygame/apps/clustersvr/src/global"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
)

var GSessionMgr = &SessionMgr{}
type SessionMgr struct {
	m_sessions_ map[uint64]*ClusterClientSession
}

func(mgr *SessionMgr) OnAccept(tcp_session *tcp.Session) {
	if mgr.m_sessions_ == nil {
		mgr.m_sessions_ = make(map[uint64]*ClusterClientSession)
	}

	logic_session := CreateSession(tcp_session, global.ClusterSvrGlobal, &ProtoMsg.PbSvrBaseInfo{
		GroupId: int32(etc.Cluster_Config.System_.Svr_group_id_),
		SvrId: int32(etc.Cluster_Config.System_.Svr_id_),
		SvrType: ProtoMsg.EmSvrType_Cluster,
		Addr: etc.Cluster_Config.System_.Svr_addr_,
		Ttl: int32(etc.Cluster_Config.System_.Svr_ttl_),
		Timeout: int32(etc.Cluster_Config.System_.Svr_timeout_),
	})
	mgr.m_sessions_[tcp_session.Id()] = logic_session.(*ClusterClientSession)
	logic_session.OnCreate()
}

func(mgr *SessionMgr) OnRecv(tcp_session *tcp.Session, msgs interface{}) {
	logic_session, ok := mgr.m_sessions_[tcp_session.Id()]
	if !ok {
		return
	}

	logic_session.OnRecv(msgs)
}

func(mgr *SessionMgr) OnClose(tcp_session *tcp.Session) {
	logic_session, ok := mgr.m_sessions_[tcp_session.Id()]
	if !ok {
		return
	}

	delete(mgr.m_sessions_, tcp_session.Id())
	logic_session.OnClose()
}

func(mgr *SessionMgr) GetSessionById(id uint64) *ClusterClientSession{
	logic_session, ok := mgr.m_sessions_[id]
	if ok {
		return logic_session
	}
	return nil
}