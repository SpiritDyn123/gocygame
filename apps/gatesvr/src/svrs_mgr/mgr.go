package svrs_mgr

import (
	"github.com/SpiritDyn123/gocygame/apps/gatesvr/src/global"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/golang/protobuf/proto"
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
}


func(mgr *svrsMgr) Start() (err error) {
	global.GateSvrGlobal.GetMsgDispatcher().Register(ProtoMsg.EmMsgId_SVR_MSG_REGISTER_GAME,
		&ProtoMsg.PbSvrRegisterGameResMsg{}, mgr.onRegSvr)
	global.GateSvrGlobal.GetMsgDispatcher().Register(ProtoMsg.EmMsgId_SVR_MSG_REGISTER_LOGIN,
		&ProtoMsg.PbSvrRegisterLoginResMsg{}, mgr.onRegSvr)

	mgr.m_svrs_info_ = make(map[ProtoMsg.EmSvrType]*svrTypeInfo)
	return
}

func(mgr *svrsMgr) Stop() {

}

func (mgr *svrsMgr) OnSvrClose(sssesion *session.SvrSession) {
	cfg_svr_info := sssesion.Config_info_

	type_info, ok := mgr.m_svrs_info_[cfg_svr_info.SvrType]
	if !ok {
		return
	}

	svr_info, ok := type_info.m_svrs_info_[cfg_svr_info.SvrId]
	if !ok {
		return
	}

	if svr_info.session_ != sssesion {
		return
	}

	svr_info.onClose()
	delete(type_info.m_svrs_info_, cfg_svr_info.SvrId)
	log.Release("OnSvrClose svr info:%+v", cfg_svr_info)
	return
}

func (mgr *svrsMgr) onRegSvr(sink interface{}, head common.IMsgHead, msg proto.Message) {
	sssesion := sink.(*session.SvrSession)
	cfg_svr_info := sssesion.Config_info_

	ret_code := int32(0)
	switch ProtoMsg.EmMsgId(head.GetMsgId()) {
	case ProtoMsg.EmMsgId_SVR_MSG_REGISTER_GAME:
		ret_code = msg.(*ProtoMsg.PbSvrRegisterGameResMsg).Ret.ErrCode
	case ProtoMsg.EmMsgId_SVR_MSG_REGISTER_LOGIN:
		ret_code = msg.(*ProtoMsg.PbSvrRegisterLoginResMsg).Ret.ErrCode
	}
	if ret_code != 0 {
		return
	}

	type_info, ok := mgr.m_svrs_info_[cfg_svr_info.SvrType]
	if !ok {
		type_info = &svrTypeInfo{
			m_svrs_info_: make(map[int32]*svrInfo),
		}
		mgr.m_svrs_info_[cfg_svr_info.SvrType] = type_info
	}

	svr_info, ok := type_info.m_svrs_info_[cfg_svr_info.SvrId]
	if ok {
		if svr_info.session_ == sssesion {
			return
		}

		svr_info.onClose() //旧的服务 处理
	}

	type_info.m_svrs_info_[cfg_svr_info.SvrId] = &svrInfo{
		session_: sssesion,
	}

	log.Release("onRegSvr svr info:%+v success", cfg_svr_info)
}