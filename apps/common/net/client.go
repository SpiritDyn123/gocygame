package net

import (
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/apps/common/global"
)

type clientInfo struct {
	cli_ *tcp.Client
	session_ interface{}
}

type CreateSessionCB func(session *tcp.Session, config_info *ProtoMsg.PbSvrBaseInfo)
type ClientMgr struct {
	m_create_session_cb_ map[ProtoMsg.EmSvrType]CreateSessionCB
	m_svr_sessions_ map[ProtoMsg.EmSvrType]map[int32]*clientInfo
}

func (mgr *ClientMgr) Init(sever global.IServerGlobal, create_cbs map[ProtoMsg.EmSvrType]CreateSessionCB) (err error) {
	mgr.m_create_session_cb_ = create_cbs

	
	return
}

func (mgr *ClientMgr) Add(svr_info *ProtoMsg.PbSvrBaseInfo) {

}

func (mgr *ClientMgr) Remove(svr_info *ProtoMsg.PbSvrBaseInfo) {

}

func (mgr *ClientMgr) SendToSvr(svr_type ProtoMsg.EmSvrType, svr_id int32) {

}

