package svr_info

import (
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/apps/common/global"
	"github.com/SpiritDyn123/gocygame/apps/common/net/strategy"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/golang/protobuf/proto"
	"github.com/SpiritDyn123/gocygame/libs/log"
)

const (
	Session_attribute_key_svr_info = "attribute_svr_info"
)

type SvrInfo struct {
	session_ global.ILogicSession
	svr_info_ *ProtoMsg.PbSvrBaseInfo
}

func NewSvrGroup(svr_type ProtoMsg.EmSvrType) *SvrGroup {
	sg := &SvrGroup{
		svr_type_:svr_type,
		m_svrs_info_: make(map[int32]*SvrInfo),
	}

	strategy_type := strategy.Hash
	var cfg interface{}
	switch sg.svr_type_ {
	case ProtoMsg.EmSvrType_Gate:
	case ProtoMsg.EmSvrType_Gs:
		strategy_type = strategy.Cons_hash
		cfg = 0
	case ProtoMsg.EmSvrType_World:
		strategy_type = strategy.Cons_hash
		cfg = 0
	case ProtoMsg.EmSvrType_DB:
		strategy_type = strategy.Cons_hash
		cfg = 0
	case ProtoMsg.EmSvrType_Login:
	}

	sg.selector_ = strategy.CreateSelector(strategy_type, cfg)

	return sg
}

type SvrGroup struct {
	svr_type_ ProtoMsg.EmSvrType
	m_svrs_info_ map[int32]*SvrInfo
	selector_ strategy.Selector
}

func (svr *SvrGroup) AddSession(logic_session global.ILogicSession, cfg_svr_info *ProtoMsg.PbSvrBaseInfo) bool {
	if cfg_svr_info.SvrType != svr.svr_type_ {
		return false
	}

	svr_info, ok := svr.m_svrs_info_[cfg_svr_info.SvrId]
	//todo 重复注册或者是旧的死链接
	if ok {
		//重复注册
		if svr_info.session_ == logic_session {
			log.Release("SvrGroup::AddSession svr info:%+v repeate register", cfg_svr_info)
			return false
		}

		//踢掉旧连接
		svr_info.session_.Close()
		svr_info.session_ = nil
	}

	logic_session.SetAttribute(Session_attribute_key_svr_info, cfg_svr_info)
	svr_info =  &SvrInfo{
		session_: logic_session,
		svr_info_: cfg_svr_info,
	}

	svr.m_svrs_info_[cfg_svr_info.SvrId] = svr_info

	svr.selector_.AddElement(cfg_svr_info.SvrId, logic_session)
	return true
}

func (svr *SvrGroup) RemoveSession(logic_session global.ILogicSession, cfg_svr_info *ProtoMsg.PbSvrBaseInfo) bool {
	if cfg_svr_info.SvrType != svr.svr_type_ {
		return false
	}

	svr_info, ok := svr.m_svrs_info_[cfg_svr_info.SvrId]
	//todo 重复注册或者是旧的死链接
	if !ok {
		return true
	}

	//可能是被踢掉的
	if svr_info.session_ != logic_session {
		return false
	}

	delete(svr.m_svrs_info_, cfg_svr_info.SvrId)

	svr.selector_.RemoveElement(cfg_svr_info.SvrId, logic_session)
	return true
}

//指定id的发送
func (svr *SvrGroup) SendToSvr(svr_id int32, head common.IMsgHead, msg proto.Message) (err error) {
	svr_info, ok := svr.m_svrs_info_[svr_id]
	//todo 重复注册或者是旧的死链接
	if !ok {
		err = fmt.Errorf("SvrGroup SendToSvr no svr:%d", svr_id)
		return
	}

	svr_info.session_.Send(head, msg)
	return
}

//不指定id的发送，key用uid
func (svr *SvrGroup) Send(key interface{}, head common.IMsgHead, msg proto.Message) (err error) {
	svr.selector_.SetElementId(key)
	s_obj := svr.selector_.Select()

	if s_obj == nil {
		err = fmt.Errorf("SvrGroup Send no svr key:%s", key)
		return
	}

	s_obj.(global.ILogicSession).Send(head, msg)
	return
}