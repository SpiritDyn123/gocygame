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
		nil, mgr.onRecvPlayerLogin) //客户端登陆消息，不解包
	global.GateSvrGlobal.GetSvrMsgParser().Register(uint32(ProtoMsg.EmCSMsgId_CS_MSG_PLAYER_LOGIN),
		&ProtoMsg.PbCsPlayerLoginResMsg{}, mgr.onRecvLoginRet) //登陆服登陆消息
	global.GateSvrGlobal.GetSvrMsgParser().Register(uint32(ProtoMsg.EmSSMsgId_SVR_MSG_GAME_LOGIN),
		&ProtoMsg.PbCsPlayerLoginResMsg{}, mgr.onRecvGameLoginRet) //游戏服登陆消息

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
		svr_type_in_: make(map[ProtoMsg.EmSvrType]int32),
		state_: player_state_connect,
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
	player := mgr.m_player_[logic_session.Id()]
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

	if player.state_ != player_state_connect {
		return
	}

	c_mhead := head.(*common.ProtocolClientHead)
	c_mhead.Uid_ = session.Id()
	_, err := global.GateSvrGlobal.GetSvrsMgr().SendBySvrType(session.Id(), ProtoMsg.EmSvrType_Login, head, msg)
	if err != nil {
		log.Error("playerMgr::onRecvPlayerLogin SendBySvrType err:%v", err)

		//todo session.Close()

		return
	}

	player.state_ = player_state_login
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
		player.state_ = player_state_connect
		player.OnRecvSvr(head, msg)
		return
	}

	//登陆Game服务器
	ihead.Msg_id_ = uint32(ProtoMsg.EmSSMsgId_SVR_MSG_GAME_LOGIN)
	_, err := global.GateSvrGlobal.GetSvrsMgr().SendBySvrType(login_res_msg.Uid, ProtoMsg.EmSvrType_Gs, head, &ProtoMsg.PbSvrGameLoginReqMsg{
		Uid: login_res_msg.Uid,
		Online: true,
	})

	if err != nil {
		log.Error("playerMgr::onRecvLoginRet SendBySvrType err:%v", err)
		player.state_ = player_state_connect
		return
	}
}

//游戏服登陆返回
func (mgr *playerMgr)onRecvGameLoginRet(sink interface{}, head common.IMsgHead, msg interface{}) {
	game_login_res_msg := msg.(*ProtoMsg.PbCsPlayerLoginResMsg)
	ihead := head.(*common.ProtocolInnerHead)
	session := sink.(common_global.ILogicSession)
	player := mgr.m_tmp_player_[ihead.Uid_lst_[0]]
	if player == nil {//玩家已经断开连接
		session.Send(&common.ProtocolInnerHead{
			Msg_id_: uint32(ProtoMsg.EmSSMsgId_SVR_MSG_GAME_LOGIN),
			Uid_lst_: []uint64{ game_login_res_msg.Uid },
		}, &ProtoMsg.PbSvrGameLoginReqMsg{
			Uid: game_login_res_msg.Uid,
			Online: false,
		})

		return
	}

	chead := &common.ProtocolClientHead{
		Msg_id_: uint32(ProtoMsg.EmCSMsgId_CS_MSG_PLAYER_LOGIN),
		Uid_: game_login_res_msg.Uid,
		Seq_: ihead.Seq_,
	}

	if game_login_res_msg.Ret.ErrCode != 0 {
		player.OnRecvSvr(chead, msg)
		player.state_ = player_state_connect
		return
	}

	//登陆成功
	delete(mgr.m_tmp_player_, ihead.Uid_lst_[0])

	player.uid_= game_login_res_msg.Uid

	mgr.m_player_[ihead.Uid_lst_[0]] = player
	mgr.m_player_by_id_[game_login_res_msg.Uid] = player

	player.OnRecvSvr(chead, msg)
	player.state_ = player_state_online
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

	log.Release("playerMgr::onPlayerEnterSvr enter svr:%+v", proto_msg)
}

func (mgr *playerMgr) OnSvrClosed(cfg_svr_info *ProtoMsg.PbSvrBaseInfo) {
	if cfg_svr_info == nil {
		return
	}

	//todo请求中的可以做一些处理
	//for _, player := range mgr.m_tmp_player_ {
	//	if svr_id, ok := player.svr_type_in_[cfg_svr_info.SvrType]; ok {
	//		if svr_id == cfg_svr_info.SvrId {
	//			player.session_.Close()
	//		}
	//	}
	//}

	switch cfg_svr_info.SvrType{
	case ProtoMsg.EmSvrType_Gs:
		//剔除该gamesvr上的玩家
		for _, player := range mgr.m_player_ {
			if svr_id, ok := player.svr_type_in_[cfg_svr_info.SvrType]; ok {
				if svr_id == cfg_svr_info.SvrId {
					player.session_.Close()
				}
			}
		}

	case ProtoMsg.EmSvrType_Login:

	}

	log.Release("playerMgr::OnSvrClosed svr_info:%+v", cfg_svr_info)
}
