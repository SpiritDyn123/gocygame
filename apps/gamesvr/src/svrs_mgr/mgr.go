package svrs_mgr

import (
	"github.com/SpiritDyn123/gocygame/apps/gamesvr/src/global"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/golang/protobuf/proto"
	common_global"github.com/SpiritDyn123/gocygame/apps/common/global"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/apps/common/net/session"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"github.com/SpiritDyn123/gocygame/apps/common/net/strategy"
)

var SvrsMgr global.ISvrsMgr
func init() {
	SvrsMgr = &svrsMgr{}
}


type svrsMgr struct {
	m_svrs_info_ map[ProtoMsg.EmSvrType]*strategy.SvrGroup

	m_tmp_session_ map[uint64]common_global.ILogicSession
}

func (mgr *svrsMgr) Start() (err error) {
	//注册消息
	global.GameSvrGlobal.GetMsgDispatcher().Register(ProtoMsg.EmMsgId_SVR_MSG_REGISTER_GAME,
		&ProtoMsg.PbSvrRegisterGameReqMsg{}, mgr.onReqRegister)

	mgr.m_svrs_info_ = make(map[ProtoMsg.EmSvrType]*strategy.SvrGroup)
	mgr.m_tmp_session_ = make(map[uint64]common_global.ILogicSession)

	return
}

func (mgr *svrsMgr) Stop() {

}

func (mgr *svrsMgr) OnAccept(tcp_session *tcp.Session) {
	logic_session := session.CreateClientSession(tcp_session, global.GameSvrGlobal, &ProtoMsg.PbSvrBaseInfo{})

	//注册关闭事件
	logic_session.(*session.ClientSession).SetSessionEventCB(session.SessionEvent_Close, func(s common_global.ILogicSession){
		mgr.onSvrOffline(s)
	})

	mgr.m_tmp_session_[logic_session.Id()] = logic_session
}

func (mgr *svrsMgr) OnRecv(tcp_session *tcp.Session, data interface{}) {
	tmp_session, ok := mgr.m_tmp_session_[tcp_session.Id()]
	if ok {
		tmp_session.OnRecv(data)
	}
}

func (mgr *svrsMgr) OnClose(tcp_session *tcp.Session) {
	tmp_session, ok := mgr.m_tmp_session_[tcp_session.Id()]
	if ok {
		tmp_session.OnClose()
		delete(mgr.m_tmp_session_, tcp_session.Id())
	}
}

func (mgr *svrsMgr) onReqRegister(sink interface{}, head common.IMsgHead, msg proto.Message) {
	req_msg := msg.(*ProtoMsg.PbSvrRegisterGameReqMsg)
	logic_session := sink.(*session.ClientSession)

	group_info, ok := mgr.m_svrs_info_[req_msg.SvrInfo.SvrType]
	if !ok {
		mgr.m_svrs_info_[req_msg.SvrInfo.SvrType] = strategy.NewSvrGroup(req_msg.SvrInfo.SvrType)
		group_info = mgr.m_svrs_info_[req_msg.SvrInfo.SvrType]
	}

	if !group_info.AddSession(logic_session, req_msg.SvrInfo) {
		return
	}

	resp_msg := &ProtoMsg.PbSvrRegisterGameResMsg{
		Ret: &ProtoMsg.Ret{
			ErrCode: 0,
		},
	}

	logic_session.Send(head, resp_msg)
	log.Release("onReqRegister svr info:%+v", req_msg.SvrInfo)
}

func (mgr *svrsMgr) onSvrOffline(s common_global.ILogicSession) {
	logic_session := s.(*session.ClientSession)
	i_svr_info := logic_session.GetAttribute(global.Session_attribute_key_svr_info)
	if i_svr_info == nil {
		return
	}

	cfg_svr_info := i_svr_info.(*ProtoMsg.PbSvrBaseInfo)

	group_info, ok := mgr.m_svrs_info_[cfg_svr_info.SvrType]
	if !ok {
		return
	}

	if !group_info.RemoveSession(logic_session, cfg_svr_info) {
		return
	}
	log.Release("onSvrOffline svr info:%+v", cfg_svr_info)
}

func (mgr *svrsMgr) SendToSvr(svr_type ProtoMsg.EmSvrType, svr_id int32, head common.IMsgHead, msg proto.Message) {

}