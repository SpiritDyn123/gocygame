package player

import (
	"github.com/SpiritDyn123/gocygame/apps/gatesvr/src/global"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/golang/protobuf/proto"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/apps/common/net/session"
	common_global"github.com/SpiritDyn123/gocygame/apps/common/global"
)

var PlayerMgr global.IPlayerMgr
func init() {
	PlayerMgr = &playerMgr{
		m_tmp_player_: make(map[uint64]*player),
		m_player_: make(map[uint64]*player),
	}
}

type playerMgr struct {
	m_tmp_player_ map[uint64]*player
	m_player_		map[uint64]*player
}

func (mgr *playerMgr) Start() (err error) {
	global.GateSvrGlobal.GetMsgDispatcher().Register(uint32(ProtoMsg.EmCSMsgId_CS_MSG_PLAYER_LOGIN),
		&ProtoMsg.PbCsPlayerLoginReqMsg{}, mgr.onRecvLogin) //登陆消息


	return
}

func (mgr *playerMgr) Stop() {

}

func (mgr *playerMgr) OnAccept(tcp_session *tcp.Session) {
	logic_session := session.CreateClientSession(tcp_session, global.GateSvrGlobal, global.GateSvrGlobal.GetSvrBaseInfo()).(*session.ClientSession)
	logic_session.SetSessionEventCB(session.SessionEvent_Close, func(s common_global.ILogicSession){
		mgr.OnPlayerOffline(s)
	})

	p := &player{
		session_: logic_session,
	}

	logic_session.OnCreate()
	mgr.m_tmp_player_[p.session_.Id()] = p
}

func (mgr *playerMgr) OnRecv(tcp_session *tcp.Session, data interface{}) {
	p := mgr.m_tmp_player_[tcp_session.Id()]
	if p != nil {
		p.session_.OnRecv(data)
		return
	}

	p = mgr.m_player_[tcp_session.Id()]
	if p != nil {
		p.session_.OnRecv(data)
	}
}

func (mgr *playerMgr) OnClose(tcp_session *tcp.Session) {
	p := mgr.m_tmp_player_[tcp_session.Id()]
	if p != nil {
		p.session_.OnClose()
		p.session_ = nil
		delete(mgr.m_tmp_player_, tcp_session.Id())
		return
	}

	p = mgr.m_player_[tcp_session.Id()]
	if p != nil {
		p.session_.OnClose()
		p.session_ = nil
		delete(mgr.m_player_, tcp_session.Id())
	}
}

func (mgr *playerMgr) OnPlayerOffline(logic_session common_global.ILogicSession) {

}

func (mgr *playerMgr) onRecvLogin(sink interface{}, head common.IMsgHead, msg proto.Message) {

}