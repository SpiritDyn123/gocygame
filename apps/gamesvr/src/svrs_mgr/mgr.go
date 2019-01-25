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
)

var SvrsMgr global.ISvrsMgr
func init() {
	SvrsMgr = &svrsMgr{}
}

type svrTypeInfo struct {
	m_svrs_info_ map[int32]*svrInfo
}

type svrsMgr struct {
	m_svrs_info_ map[ProtoMsg.EmSvrType]*svrTypeInfo

	m_tmp_session_ map[uint64]common_global.ILogicSession
}

func (mgr *svrsMgr) Start() (err error) {
	//注册消息
	global.GameSvrGlobal.GetMsgDispatcher().Register(ProtoMsg.EmMsgId_SVR_MSG_REGISTER_GAME,
		&ProtoMsg.PbSvrRegisterGameReqMsg{}, mgr.onReqRegister)

	mgr.m_svrs_info_ = make(map[ProtoMsg.EmSvrType]*svrTypeInfo)
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

	type_info, ok := mgr.m_svrs_info_[req_msg.SvrInfo.SvrType]
	if !ok {
		mgr.m_svrs_info_[req_msg.SvrInfo.SvrType] = &svrTypeInfo{
			m_svrs_info_: make(map[int32]*svrInfo),
		}
		type_info = mgr.m_svrs_info_[req_msg.SvrInfo.SvrType]
	}

	svr_info, ok := type_info.m_svrs_info_[req_msg.SvrInfo.SvrId]
	//todo 重复注册或者是旧的死链接
	if ok {
		//重复注册
		if svr_info.session_ == logic_session {
			log.Release("onReqRegister svr info:%+v repeate register", req_msg.SvrInfo)
			return
		}

		//踢掉旧连接
		svr_info.session_.Close()
		svr_info.session_ = nil
	}

	logic_session.SetAttribute(global.Session_attribute_key_svr_info, req_msg.SvrInfo)
	svr_info =  &svrInfo{
		session_: logic_session,
		svr_info_: req_msg.SvrInfo,
	}

	type_info.m_svrs_info_[req_msg.SvrInfo.SvrId] = svr_info

	resp_msg := &ProtoMsg.PbSvrRegisterGameResMsg{
		Ret: &ProtoMsg.Ret{
			ErrCode: 0,
		},
	}

	svr_info.session_.Send(head, resp_msg)
	log.Release("onReqRegister svr info:%+v", req_msg.SvrInfo)
}

func (mgr *svrsMgr) onSvrOffline(s common_global.ILogicSession) {
	logic_session := s.(*session.ClientSession)
	i_svr_info := logic_session.GetAttribute(global.Session_attribute_key_svr_info)
	if i_svr_info == nil {
		return
	}

	cfg_svr_info := i_svr_info.(*ProtoMsg.PbSvrBaseInfo)

	type_info, ok := mgr.m_svrs_info_[cfg_svr_info.SvrType]
	if !ok {
		return
	}

	svr_info, ok := type_info.m_svrs_info_[cfg_svr_info.SvrId]
	//todo 重复注册或者是旧的死链接
	if !ok {
		return
	}

	//可能是被踢掉的
	if svr_info.session_ != s {
		return
	}

	delete(type_info.m_svrs_info_, cfg_svr_info.SvrId)
	log.Release("onSvrOffline svr info:%+v", cfg_svr_info)
}

func (mgr *svrsMgr) SendToSvr(svr_type ProtoMsg.EmSvrType, svr_id int32, head common.IMsgHead, msg proto.Message) {
	type_info, ok := mgr.m_svrs_info_[svr_type]
	if !ok {
		return
	}

	svr_info, ok := type_info.m_svrs_info_[svr_id]
	if !ok {
		return
	}

	if svr_info.session_ == nil {
		return
	}

	svr_info.session_.Send(head, msg)
}