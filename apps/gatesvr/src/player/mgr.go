package player

import (
	"github.com/SpiritDyn123/gocygame/apps/gatesvr/src/global"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/apps/common/net/session"
	common_global"github.com/SpiritDyn123/gocygame/apps/common/global"
	"github.com/SpiritDyn123/gocygame/libs/log"
)

var PlayerMgr global.IPlayerMgr
func init() {
	PlayerMgr = &playerMgr{
		m_tmp_player_: make(map[uint64]*player),
		m_player_: make(map[uint64]*player),
		m_player_by_id_: make(map[uint64]*player),
	}
}

type playerMgr struct {
	m_tmp_player_ map[uint64]*player
	m_player_		map[uint64]*player
	m_player_by_id_ map[uint64]*player
}

func (mgr *playerMgr) Start() (err error) {
	global.GateSvrGlobal.GetMsgDispatcher().Register(uint32(ProtoMsg.EmCSMsgId_CS_MSG_PLAYER_LOGIN),
		&ProtoMsg.PbCsPlayerLoginReqMsg{}, mgr.onRecvPlayerLogin) //登陆消息
	global.GateSvrGlobal.GetSvrMsgParser().Register(uint32(ProtoMsg.EmCSMsgId_CS_MSG_PLAYER_LOGIN),
		&ProtoMsg.PbCsPlayerLoginReqMsg{}, mgr.onRecvLoginRet) //登陆消息

	global.GateSvrGlobal.GetSvrMsgParser().Register(uint32(ProtoMsg.EmSSMsgId_SVR_MSG_COMMON_PUSH_PLAYER_SVRID),
		&ProtoMsg.PbSvrCommonPushPlayerSvrId{}, mgr.onPlayerEnterSvr) //玩家进入服务的id

	return
}

func (mgr *playerMgr) Stop() {

}

func (mgr *playerMgr) OnAccept(tcp_session *tcp.Session) {
	logic_session := session.CreateClientSession(tcp_session, global.GateSvrGlobal, global.GateSvrGlobal.GetMsgDispatcher(),
		global.GateSvrGlobal.GetSvrBaseInfo()).(*session.ClientSession)
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

func (mgr *playerMgr) GetPlayerById(uid uint64) global.IPlayer {
	return mgr.m_player_by_id_[uid]
}

func (mgr *playerMgr) BroadClientMsg(head common.IMsgHead, msg interface{}) {
	mhead := head.(*common.ProtocolInnerHead)
	for _, p := range mgr.m_player_by_id_ {
		cli_mhead := &common.ProtocolClientHead{
			Msg_id_: mhead.Msg_id_,
			Seq_: mhead.Seq_,
			Uid_: p.uid_,
		}
		p.session_.Send(cli_mhead, msg)
	}
}

func (mgr *playerMgr) onRecvPlayerLogin(sink interface{}, head common.IMsgHead, msg interface{}) {
	session := sink.(common_global.ILogicSession)
	player := mgr.m_player_[session.Id()]

	if player.uid_ != 0 {
		return
	}

	_, err := global.GateSvrGlobal.GetSvrsMgr().SendBySvrType(session.Id(), ProtoMsg.EmSvrType_Login, head, msg)
	if err != nil {
		log.Error("playerMgr::onRecvLogin SendBySvrType err:%v", err)

		//todo session.Close()

		return
	}


}

func (mgr *playerMgr) onRecvLoginRet(sink interface{}, head common.IMsgHead, msg interface{}) {


}

func (mgr *playerMgr) onPlayerEnterSvr(sink interface{}, head common.IMsgHead, msg interface{}) {
	proto_msg := msg.(*ProtoMsg.PbSvrCommonPushPlayerSvrId)
	p := mgr.m_player_by_id_[proto_msg.Uid]
	if p == nil {
		return
	}

	if proto_msg.Add {
		p.svr_type_in_[proto_msg.SvrType] = proto_msg.SvrId
	} else {
		delete(p.svr_type_in_, proto_msg.SvrType)
	}
}