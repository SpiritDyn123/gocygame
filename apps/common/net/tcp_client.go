package net

import (
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/common/global"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/libs/chanrpc"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"time"
)

type clientInfo struct {
	cli_ *tcp.Client
	svr_info_ *ProtoMsg.PbSvrBaseInfo
	session_ global.ILogicSession
}

type CreateSessionCB func(session *tcp.Session, svr_global global.IServerGlobal, config_info *ProtoMsg.PbSvrBaseInfo) global.ILogicSession
type TcpClientMgr struct {
	Msg_parser_ tcp.IMsgParser
	Protocol_  tcp.Protocol
	Send_chan_size_ int
	Chan_server_   *chanrpc.Server
	Connect_key_  string
	Close_key_  string
	Recv_key_  string
	Connect_interval_ time.Duration
	Dial_timeout_ time.Duration
	Tls_   bool

	Svr_global_ global.IServerGlobal
	M_create_session_cb_ map[ProtoMsg.EmSvrType]CreateSessionCB

	m_svr_sessions_ map[ProtoMsg.EmSvrType]map[int32]*clientInfo
}

func (mgr *TcpClientMgr) Start() (err error) {
	mgr.Chan_server_.Register(mgr.Connect_key_, mgr.onSessionConnect)
	mgr.Chan_server_.Register(mgr.Close_key_, mgr.onSessionClose)
	mgr.Chan_server_.Register(mgr.Recv_key_, mgr.onSessionRecv)

	mgr.m_svr_sessions_ = make(map[ProtoMsg.EmSvrType]map[int32]*clientInfo)
	return
}

func (mgr *TcpClientMgr) Stop() {
	for _, type_info := range mgr.m_svr_sessions_ {
		for _, client_info := range type_info {
			client_info.cli_.Stop()
		}
	}
}

//添加新的服务器连接
func (mgr *TcpClientMgr) AddClient(svr_info *ProtoMsg.PbSvrBaseInfo) (err error){
	if svr_info == nil {
		return
	}
	
	_, ok := mgr.M_create_session_cb_[svr_info.SvrType]
	if !ok {
		return fmt.Errorf("ClientMgr::AddClient has no CreateSessionCB")
	}

	type_info, ok := mgr.m_svr_sessions_[svr_info.SvrType]
	if !ok {
		type_info = make(map[int32]*clientInfo)
		mgr.m_svr_sessions_[svr_info.SvrType] = type_info
	}

	client_info, ok := type_info[svr_info.SvrId]
	if ok {
		if client_info.svr_info_.Addr == svr_info.Addr { //如果tcp假死，可能需要等待心跳超时自动重连(有延迟效果)
			return
		}

		if client_info.session_ != nil {
			client_info.session_.OnClose()
		}

		client_info.cli_.Stop()
	}

	type_info[svr_info.SvrId] = &clientInfo{
		cli_: &tcp.Client{
			AutoReconnect: true,
			ConnectInterval: mgr.Connect_interval_,

			Network: "tcp",
			Addr: svr_info.Addr,
			DialTimeOut: mgr.Dial_timeout_,
			ConnectNum: 1,
			Protocol: mgr.Protocol_,
			MsgParser: mgr.Msg_parser_,
			SendChanSize: mgr.Send_chan_size_,

			ChanServer: mgr.Chan_server_,
			ConnectKey: mgr.Connect_key_,
			ClosedKey: mgr.Close_key_,
			RecvKey: mgr.Recv_key_,
			UseTLS: mgr.Tls_,
			Data: svr_info,
		},

		svr_info_: svr_info,
	}

	type_info[svr_info.SvrId].cli_.Connect()
	return
}

func (mgr *TcpClientMgr) RemoveClient(svr_info *ProtoMsg.PbSvrBaseInfo) {
	if svr_info == nil {
		return
	}


	type_info, ok := mgr.m_svr_sessions_[svr_info.SvrType]
	if !ok {
		return
	}

	client_info, ok := type_info[svr_info.SvrId]
	if !ok {
		return
	}

	delete(type_info, svr_info.SvrId)
	if client_info.session_ != nil {
		client_info.session_.OnClose()
	}
	client_info.cli_.Stop()
}

func (mgr *TcpClientMgr) onSessionConnect(args []interface{}) {
	tcp_session := args[0].(*tcp.Session)
	svr_info := tcp_session.Data().(*ProtoMsg.PbSvrBaseInfo)
	type_info, ok := mgr.m_svr_sessions_[svr_info.SvrType]
	if !ok {
		tcp_session.Close()
		return
	}

	client_info, ok := type_info[svr_info.SvrId]

	//已经被删除
	if !ok || client_info.svr_info_.Addr != svr_info.Addr {
		tcp_session.Close()
		return
	}

	create_session_fun := mgr.M_create_session_cb_[svr_info.SvrType]
	logic_session := create_session_fun(tcp_session, mgr.Svr_global_, svr_info)
	client_info.session_ = logic_session
	logic_session.OnCreate()
}

func (mgr *TcpClientMgr) onSessionClose(args []interface{}) {
	tcp_session := args[0].(*tcp.Session)
	svr_info := tcp_session.Data().(*ProtoMsg.PbSvrBaseInfo)
	type_info, ok := mgr.m_svr_sessions_[svr_info.SvrType]
	if !ok {
		return
	}

	client_info, ok := type_info[svr_info.SvrId]

	//已经被删除
	if !ok || client_info.svr_info_.Addr != svr_info.Addr {
		return
	}

	client_info.session_.OnClose()
	client_info.session_ = nil
	//delete(type_info, svr_info.SvrId)
}

func (mgr *TcpClientMgr) onSessionRecv(args []interface{}) {
	tcp_session := args[0].(*tcp.Session)
	svr_info := tcp_session.Data().(*ProtoMsg.PbSvrBaseInfo)
	type_info, ok := mgr.m_svr_sessions_[svr_info.SvrType]
	if !ok {
		tcp_session.Close()
		return
	}

	client_info, ok := type_info[svr_info.SvrId]

	//已经被删除
	if !ok || client_info.svr_info_.Addr != svr_info.Addr {
		tcp_session.Close()
		return
	}

	client_info.session_.OnRecv(args[1])
}

func (mgr *TcpClientMgr) SendToSvr(svr_type ProtoMsg.EmSvrType, svr_id int32) {

}


