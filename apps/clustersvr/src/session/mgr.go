package session

import "github.com/SpiritDyn123/gocygame/libs/net/tcp"

type SessionMgr struct {
	m_sessions_ map[uint64]*ClientSession
}

func(mgr *SessionMgr) OnAccept(tcp_session *tcp.Session) {
	if mgr.m_sessions_ == nil {
		mgr.m_sessions_ = make(map[uint64]*ClientSession)
	}

	logic_session := CreateSession(tcp_session)
	mgr.m_sessions_[tcp_session.Id()] = logic_session

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

	logic_session.OnClose()
}