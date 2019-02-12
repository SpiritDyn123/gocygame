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

type playerState int
const (
	player_state_login = playerState(1) + iota
	player_state_login_suc
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
		nil, mgr.onRecvPlayerLogin) //客户端登陆消息，不解包
	global.GateSvrGlobal.GetSvrMsgParser().Register(uint32(ProtoMsg.EmCSMsgId_CS_MSG_PLAYER_LOGIN),
		&ProtoMsg.PbCsPlayerLoginResMsg{}, mgr.onRecvLoginRet) //登陆服登陆消息
	global.GateSvrGlobal.GetSvrMsgParser().Register(uint32(ProtoMsg.EmSSMsgId_SVR_MSG_GAME_LOGIN),
		&ProtoMsg.PbSvrRegisterGameResMsg{}, mgr.onRecvGameLoginRet) //游戏服登陆消息

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
		delete(mgr.m_player_by_id_, p.uid_)
	}
}

func (mgr *playerMgr) OnPlayerOffline(logic_session common_global.ILogicSession) {
	session := logic_session.(common_global.ILogicSession)
	player := mgr.m_player_[session.Id()]
	if player == nil {
		return
	}

	//通知Game服务器 玩家下线
	if gs_id, ok := player.svr_type_in_[ProtoMsg.EmSvrType_Gs]; ok {
		global.GateSvrGlobal.GetSvrsMgr().SendBySvrId(ProtoMsg.EmSvrType_Gs, gs_id, &common.ProtocolInnerHead{
			Msg_id_: uint32(ProtoMsg.EmSSMsgId_SVR_MSG_GAME_LOGIN),
			Uid_lst_: []uint64{ player.uid_ },
		}, &ProtoMsg.PbSvrGameLoginReqMsg{
			Uid: player.uid_,
			Online: false,
		})
	}
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
	player := mgr.m_tmp_player_[session.Id()]

	if player == nil {
		session.Close()
		return
	}

	//防止重新发登陆消息
	player = mgr.m_player_[session.Id()]
	if player != nil {
		return
	}

	c_mhead := head.(*common.ProtocolClientHead)
	c_mhead.Uid_ = session.Id()
	_, err := global.GateSvrGlobal.GetSvrsMgr().SendBySvrType(session.Id(), ProtoMsg.EmSvrType_Login, head, msg)
	if err != nil {
		log.Error("playerMgr::onRecvLogin SendBySvrType err:%v", err)

		//todo session.Close()

		return
	}


}

//登陆服登陆返回
func (mgr *playerMgr) onRecvLoginRet(sink interface{}, head common.IMsgHead, msg interface{}) {
	login_res_msg := msg.(*ProtoMsg.PbCsPlayerLoginResMsg)
	ihead := head.(*common.ProtocolInnerHead)
	player := mgr.m_tmp_player_[ihead.Uid_lst_[0]]
	if player == nil {
		return
	}

	if login_res_msg.Ret.ErrCode != 0 {
		player.session_.Send(common.InnerToClientHead(head), msg)
		return
	}

	//登陆Game服务器
	global.GateSvrGlobal.GetSvrsMgr().SendBySvrType(login_res_msg.Uid, ProtoMsg.EmSvrType_Gs, head, &ProtoMsg.PbSvrGameLoginReqMsg{
		Uid: login_res_msg.Uid,
		Online: true,
	})
}

//游戏服登陆返回
func (mgr *playerMgr)onRecvGameLoginRet(sink interface{}, head common.IMsgHead, msg interface{}) {
	game_login_res_msg := msg.(*ProtoMsg.PbSvrGameLoginResMsg)
	ihead := head.(*common.ProtocolInnerHead)
	player := mgr.m_tmp_player_[ihead.Uid_lst_[0]]
	if player == nil {
		return
	}

	if game_login_res_msg.Ret.ErrCode != 0 {
		player.session_.Send(common.ClientToInnerHead(head), msg)
		return
	}

	//登陆成功
	delete(mgr.m_tmp_player_, ihead.Uid_lst_[0])

	player.uid_= game_login_res_msg.Uid

	mgr.m_player_[ihead.Uid_lst_[0]] = player
	mgr.m_player_by_id_[game_login_res_msg.Uid] = player
	player.session_.Send(common.ClientToInnerHead(head), msg)

	log.Release("playerMgr::onRecvGameLoginRet player:%d login success", player.uid_)
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