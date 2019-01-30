package session

import (
	"github.com/SpiritDyn123/gocygame/apps/common/global"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"time"
	"fmt"
)

type SessionEvent int
const (
	SessionEvent_Accept = SessionEvent(1) + iota
	SessionEvent_Close
	SessionEvent_Recv
)

type SessionEventCallBack func(global.ILogicSession)
type BaseSession struct {
	*tcp.Session
	Last_check_time_ time.Time

	Svr_global_  global.IServerGlobal
	wtId_ uint64	//心跳定时器 id
	closed_ bool

	Config_info_ *ProtoMsg.PbSvrBaseInfo

	M_event_cbs_ map[SessionEvent]SessionEventCallBack

	m_attribute_ map[string]interface{}
}

func (session *BaseSession) Send(msg ...interface{}) error {
	if session == nil || session.Session == nil {
		return fmt.Errorf("send in closed client session")
	}
	return session.Session.Send(msg...)
}

func(session *BaseSession) OnRecv(data interface{})(now time.Time, is_hb bool) {
	return
}

func(session *BaseSession) OnClose() {

}

func(session *BaseSession) OnCreate() {

}

func(session *BaseSession) SetAttribute(key string, value interface{}) {
	session.m_attribute_[key] = value
}

func(session *BaseSession) GetAttribute(key string) interface{} {
	if session.m_attribute_ == nil {
		return nil
	}

	value, ok := session.m_attribute_[key]
	if ok {
		return value
	}
	return nil
}

func(session *BaseSession) SetSessionEventCB(ev_type SessionEvent, cb SessionEventCallBack) {
	if session.m_attribute_ == nil {
		session.m_attribute_ = make(map[string]interface{})
	}

	session.M_event_cbs_[ev_type] = cb
}
