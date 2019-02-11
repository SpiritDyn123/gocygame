package net

import (
	"encoding/binary"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/apps/common/dbengine"
	"github.com/SpiritDyn123/gocygame/apps/common/global"
	"github.com/SpiritDyn123/gocygame/apps/common/net/codec"
	"github.com/SpiritDyn123/gocygame/apps/common/net/session"
	"github.com/SpiritDyn123/gocygame/apps/common/net/svr_info"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/golang/protobuf/proto"
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/common/tools"
)

type SvrsMgr struct {
	Svr_global_ global.IServerGlobal
	Publish_svrs_ []ProtoMsg.EmSvrType
	Cluster_svr_info_ *common.Cfg_Json_Svr_Item

	Client_msg_dispatcher_ tools.IMsgDispatcher
	Svr_msg_dispatcher_ tools.IMsgDispatcher

	//回调函数
	CB_Registered_ 	func(svr_info *ProtoMsg.PbSvrBaseInfo)
	CB_Closed_ 		func(svr_info *ProtoMsg.PbSvrBaseInfo)

	cluster_ *TcpClientCluster

	m_svrs_info_ map[ProtoMsg.EmSvrType]*svr_info.SvrGroup

	m_tmp_session_ map[uint64]global.ILogicSession

	dbengin_client_ *dbengine.DBEngineClient //db类型可以抽象出来redisclient
}

func (mgr *SvrsMgr) hasSvr(svr_type ProtoMsg.EmSvrType) bool {
	for _, stype := range mgr.Publish_svrs_ {
		if stype == svr_type {
			return true
		}
	}
	return false
}

func (mgr *SvrsMgr) Start() (err error) {

	cur_svr_type := mgr.Svr_global_.GetSvrBaseInfo().SvrType
	//注册消息
	if mgr.hasSvr(ProtoMsg.EmSvrType_Gs) {
		mgr.Svr_msg_dispatcher_.Register(uint32(ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_GAME),
			&ProtoMsg.PbSvrRegisterGameResMsg{}, mgr.onResRegister)
	} else {
		mgr.Client_msg_dispatcher_.Register(uint32(ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_GAME),
			&ProtoMsg.PbSvrRegisterGameReqMsg{}, mgr.onReqRegister)
	}

	if mgr.hasSvr(ProtoMsg.EmSvrType_DB) {
		mgr.Svr_msg_dispatcher_.Register(uint32(ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_DB),
			&ProtoMsg.PbSvrRegisterDBResMsg{}, mgr.onResRegister)
	} else {
		mgr.Client_msg_dispatcher_.Register(uint32(ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_DB),
			&ProtoMsg.PbSvrRegisterDBReqMsg{}, mgr.onReqRegister)
	}

	if mgr.hasSvr(ProtoMsg.EmSvrType_World) {
		mgr.Svr_msg_dispatcher_.Register(uint32(ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_WORLD),
			&ProtoMsg.PbSvrRegisterWorldResMsg{}, mgr.onResRegister)
	} else {
		mgr.Client_msg_dispatcher_.Register(uint32(ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_WORLD),
			&ProtoMsg.PbSvrRegisterWorldReqMsg{}, mgr.onReqRegister)
	}

	if mgr.hasSvr(ProtoMsg.EmSvrType_Login) {
		mgr.Svr_msg_dispatcher_.Register(uint32(ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_LOGIN),
			&ProtoMsg.PbSvrRegisterLoginResMsg{}, mgr.onResRegister)
	} else {
		mgr.Client_msg_dispatcher_.Register(uint32(ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_LOGIN),
			&ProtoMsg.PbSvrRegisterLoginReqMsg{}, mgr.onReqRegister)
	}


	mgr.m_svrs_info_ = make(map[ProtoMsg.EmSvrType]*svr_info.SvrGroup)
	mgr.m_tmp_session_ = make(map[uint64]global.ILogicSession)

	//db客户端
	if cur_svr_type != ProtoMsg.EmSvrType_DB {
		mgr.dbengin_client_ = &dbengine.DBEngineClient{
			Svr_global_: mgr.Svr_global_,
		}
		if err = mgr.dbengin_client_.Start(); err != nil {
			return
		}
	}

	//集群管理器
	clus_msg_parser := tcp.NewMsgParser()
	clus_msg_parser.SetByteOrder(common.Default_Net_Endian == binary.LittleEndian)
	clus_msg_parser.SetIncludeHead(true)
	clus_msg_parser.SetMsgLen(common.Default_Net_Head_Len, common.Default_Svr_Recv_len, common.Default_Svr_Send_len)
	mgr.cluster_ = &TcpClientCluster{
		TcpClientMgr: TcpClientMgr{
			Msg_parser_: clus_msg_parser,
			Protocol_ : &codec.ProtoInnerProtocol{ Endian_: common.Default_Net_Endian },
			Send_chan_size_: common.Default_Svr_Send_Chan_Len,
			Chan_server_: mgr.Svr_global_.GetChanServer(),
			Connect_key_: common.Chanrpc_key_tcp_inner_accept,
			Close_key_: common.Chanrpc_key_tcp_inner_close,
			Recv_key_ : common.Chanrpc_key_tcp_inner_recv,
			Tls_: false,
			Svr_global_: mgr.Svr_global_,
			M_create_session_cb_: map[ProtoMsg.EmSvrType]CreateSessionCB{},
		},
		Cluster_svr_info_: mgr.Cluster_svr_info_,
		Svr_info_: mgr.Svr_global_.GetSvrBaseInfo(),
		Publish_svrs_: mgr.Publish_svrs_,
		Msg_dispatcher_: mgr.Svr_msg_dispatcher_,
	}

	for _, svr_type := range mgr.Publish_svrs_ {
		mgr.cluster_.M_create_session_cb_[svr_type] = mgr.createInnerSvrSession
	}

	if err = mgr.cluster_.Start(); err != nil {
		return
	}


	return
}

func (mgr *SvrsMgr) Stop() {
	if mgr.cluster_ != nil {
		mgr.cluster_.Stop()
	}
}

func (mgr *SvrsMgr) OnAccept(tcp_session *tcp.Session) {
	logic_session := session.CreateClientSession(tcp_session, mgr.Svr_global_, mgr.Client_msg_dispatcher_,
		mgr.Svr_global_.GetSvrBaseInfo())

	//注册关闭事件
	logic_session.(*session.ClientSession).SetSessionEventCB(session.SessionEvent_Close, func(s global.ILogicSession){
		mgr.OnSvrOffline(s)
	})

	logic_session.OnCreate()

	mgr.m_tmp_session_[logic_session.Id()] = logic_session
}

func (mgr *SvrsMgr) OnRecv(tcp_session *tcp.Session, data interface{}) {
	tmp_session, ok := mgr.m_tmp_session_[tcp_session.Id()]
	if ok {
		tmp_session.OnRecv(data)
	}
}

func (mgr *SvrsMgr) OnClose(tcp_session *tcp.Session) {
	tmp_session, ok := mgr.m_tmp_session_[tcp_session.Id()]
	if ok {
		tmp_session.OnClose()
		delete(mgr.m_tmp_session_, tcp_session.Id())
	}
}

func (mgr *SvrsMgr) SendBySvrType(key interface{}, svr_type ProtoMsg.EmSvrType, head common.IMsgHead, msg interface{}) (svr_id int32, err error) {
	sg := mgr.m_svrs_info_[svr_type]
	if sg == nil {
		err = fmt.Errorf("SvrsMgr::SendBySvrType has no svr group:%v", svr_type)
		return
	}

	return sg.Send(key, head, msg)
}

func (mgr *SvrsMgr) SendBySvrId(svr_type ProtoMsg.EmSvrType, svr_id int32, head common.IMsgHead, msg interface{}) (err error) {
	sg := mgr.m_svrs_info_[svr_type]

	if sg == nil {
		err = fmt.Errorf("SvrsMgr::SendBySvrId has no svr group:%v", svr_type)
		return
	}

	return sg.SendToSvr(svr_id, head, msg)
}

func (mgr *SvrsMgr) GetSvrTypeCSId(cs_msg_id uint32) (ProtoMsg.EmSvrType) {
	cs_id := ProtoMsg.EmCSMsgId(cs_msg_id)
	for _, sg := range mgr.m_svrs_info_ {
		cfg_info := sg.GetSvrInfo()
		if cfg_info != nil {
			if cfg_info.CsMsgBegin <= cs_id && cfg_info.CsMsgEnd >= cs_id {
				return cfg_info.SvrType
			}
		}
	}

	return ProtoMsg.EmSvrType_ST_Invalid
}

func (mgr *SvrsMgr) SendByCSId(key interface{}, head common.IMsgHead, msg interface{}) (svr_type ProtoMsg.EmSvrType,
	svr_id int32, err error) {
	cs_id := ProtoMsg.EmCSMsgId(head.GetMsgId())
	for _, sg := range mgr.m_svrs_info_ {
		cfg_info := sg.GetSvrInfo()
		if cfg_info != nil {
			if cfg_info.CsMsgBegin <= cs_id && cfg_info.CsMsgEnd >= cs_id {
				svr_type = cfg_info.SvrType
				svr_id, err = sg.Send(key, head, msg)
				return
			}
		}
	}

	err = fmt.Errorf("SvrsMgr::SendByCSId has no svr group")
	return
}

func (mgr *SvrsMgr) SendBySSId(key interface{}, head common.IMsgHead, msg interface{}) (svr_type ProtoMsg.EmSvrType,
	svr_id int32, err error) {
	ss_id := ProtoMsg.EmSSMsgId(head.GetMsgId())
	for _, sg := range mgr.m_svrs_info_ {
		cfg_info := sg.GetSvrInfo()
		if cfg_info != nil {
			if cfg_info.SsMsgBegin <= ss_id && cfg_info.SsMsgEnd >= ss_id {
				svr_type = cfg_info.SvrType
				svr_id, err = sg.Send(key, head, msg)
				return
			}
		}
	}

	err = fmt.Errorf("SvrsMgr::SendBySSId has no svr group")
	return
}


func (mgr *SvrsMgr) onReqRegister(sink interface{}, head common.IMsgHead, msg interface{}) {

	var resp_msg proto.Message
	var cfg_svr_info *ProtoMsg.PbSvrBaseInfo
	switch ProtoMsg.EmSSMsgId(head.GetMsgId()) {
	case ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_DB:
		cfg_svr_info = msg.(*ProtoMsg.PbSvrRegisterDBReqMsg).SvrInfo
		resp_msg = &ProtoMsg.PbSvrRegisterDBResMsg{
			Ret: &ProtoMsg.Ret{
				ErrCode: 0,
			},
		}
	case ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_WORLD:
		cfg_svr_info = msg.(*ProtoMsg.PbSvrRegisterWorldReqMsg).SvrInfo
		resp_msg = &ProtoMsg.PbSvrRegisterWorldResMsg{
			Ret: &ProtoMsg.Ret{
				ErrCode: 0,
			},
		}
	case ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_GAME:
		cfg_svr_info = msg.(*ProtoMsg.PbSvrRegisterGameReqMsg).SvrInfo
		resp_msg = &ProtoMsg.PbSvrRegisterGameResMsg{
			Ret: &ProtoMsg.Ret{
				ErrCode: 0,
			},
		}
	case ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_LOGIN:
		cfg_svr_info = msg.(*ProtoMsg.PbSvrRegisterLoginReqMsg).SvrInfo
		resp_msg = &ProtoMsg.PbSvrRegisterLoginResMsg{
			Ret: &ProtoMsg.Ret{
				ErrCode: 0,
			},
		}
	}

	logic_session := sink.(*session.ClientSession)

	group_info, ok := mgr.m_svrs_info_[cfg_svr_info.SvrType]
	if !ok {
		mgr.m_svrs_info_[cfg_svr_info.SvrType] = svr_info.NewSvrGroup(cfg_svr_info.SvrType)
		group_info = mgr.m_svrs_info_[cfg_svr_info.SvrType]
	}

	if !group_info.AddSession(logic_session, cfg_svr_info) {
		return
	}


	logic_session.Send(head, resp_msg)

	if mgr.CB_Registered_ != nil {
		mgr.CB_Registered_(cfg_svr_info)
	}

	log.Release("onReqRegister svr info:%+v", cfg_svr_info)
}

func (mgr *SvrsMgr) onResRegister(sink interface{}, head common.IMsgHead, msg interface{}) {

	sssesion := sink.(*session.SvrSession)
	cfg_svr_info := sssesion.Config_info_
	ret_code := int32(0)
	is_db := false
	switch ProtoMsg.EmSSMsgId(head.GetMsgId()) {
	case ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_DB:
		is_db = true
		ret_code = msg.(*ProtoMsg.PbSvrRegisterDBResMsg).Ret.ErrCode
	case ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_WORLD:
		ret_code = msg.(*ProtoMsg.PbSvrRegisterWorldResMsg).Ret.ErrCode
	case ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_GAME:
		ret_code = msg.(*ProtoMsg.PbSvrRegisterGameResMsg).Ret.ErrCode
	case ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_LOGIN:
		ret_code = msg.(*ProtoMsg.PbSvrRegisterLoginResMsg).Ret.ErrCode
	}
	if ret_code != 0 {
		return
	}

	group_info, ok := mgr.m_svrs_info_[cfg_svr_info.SvrType]
	if !ok {
		mgr.m_svrs_info_[cfg_svr_info.SvrType] = svr_info.NewSvrGroup(cfg_svr_info.SvrType)
		group_info = mgr.m_svrs_info_[cfg_svr_info.SvrType]
	}

	if !group_info.AddSession(sssesion, cfg_svr_info) {
		return
	}

	//dbengine客户端更新链接
	if is_db {
		mgr.dbengin_client_.Svr_group_ = group_info
	}

	if mgr.CB_Registered_ != nil {
		mgr.CB_Registered_(cfg_svr_info)
	}

	log.Release("onResRegister svr info:%+v", sssesion)
}

func (mgr *SvrsMgr) OnSvrOffline(logic_session global.ILogicSession) {
	i_svr_info := logic_session.GetAttribute(svr_info.Session_attribute_key_svr_info)
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

	//dbengine客户端更新链接
	if cfg_svr_info.SvrType == ProtoMsg.EmSvrType_DB {
		mgr.dbengin_client_.Svr_group_ = group_info
	}

	if mgr.CB_Closed_ != nil {
		mgr.CB_Closed_(cfg_svr_info)
	}

	log.Release("OnSvrOffline svr info:%+v", cfg_svr_info)
}

func (mgr *SvrsMgr) createInnerSvrSession(tcp_session *tcp.Session, svr_global global.IServerGlobal,
	config_info *ProtoMsg.PbSvrBaseInfo) global.ILogicSession {
	cs := session.CreateSvrSession(tcp_session, svr_global, mgr.Svr_msg_dispatcher_, config_info)

	//注册连接事件
	cs.(*session.SvrSession).SetSessionEventCB(session.SessionEvent_Accept, mgr.regSvrCallBack)
	cs.(*session.SvrSession).SetSessionEventCB(session.SessionEvent_Close, func(s global.ILogicSession){
		mgr.OnSvrOffline(s.(*session.SvrSession))
	})

	return cs
}

func (mgr *SvrsMgr) regSvrCallBack(s global.ILogicSession){
	ssession := s.(*session.SvrSession)

	//注册服务器
	m_head := &common.ProtocolInnerHead{
		Msg_id_: uint32(ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_DB),
	}

	var m_body proto.Message
	switch ssession.Config_info_.SvrType{
	case ProtoMsg.EmSvrType_DB:
		m_head.Msg_id_ = uint32(ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_DB)
		m_body =  &ProtoMsg.PbSvrRegisterDBReqMsg{
			SvrInfo: mgr.Svr_global_.GetSvrBaseInfo(),
		}
	case ProtoMsg.EmSvrType_Gs:
		m_head.Msg_id_ = uint32(ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_GAME)
		m_body =  &ProtoMsg.PbSvrRegisterGameReqMsg{
			SvrInfo: mgr.Svr_global_.GetSvrBaseInfo(),
		}
	case ProtoMsg.EmSvrType_Login:
		m_head.Msg_id_ = uint32(ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_LOGIN)
		m_body =  &ProtoMsg.PbSvrRegisterLoginReqMsg{
			SvrInfo: mgr.Svr_global_.GetSvrBaseInfo(),
		}
	case ProtoMsg.EmSvrType_World:
		m_head.Msg_id_ = uint32(ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_WORLD)
		m_body =  &ProtoMsg.PbSvrRegisterWorldReqMsg{
			SvrInfo: mgr.Svr_global_.GetSvrBaseInfo(),
		}
	default:
		return
	}

	ssession.Send(m_head, m_body)
}

func (mgr *SvrsMgr) GetDBEngineClient() *dbengine.DBEngineClient {
	return mgr.dbengin_client_
}
