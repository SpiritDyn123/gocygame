package svrs_mgr

import (
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/clustersvr/src/global"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/libs/log"
	common_session"github.com/SpiritDyn123/gocygame/apps/common/net/session"
)

var SvrsMgr global.ISvrsMgr
func init() {
	SvrsMgr = &svrsMgr{
		m_svrs_info_: make(SVR_GROUP_INFO),
		m_publish_info_: make(map[string][]ProtoMsg.EmSvrType),
	}
}

type svrInfo  struct {
	svrs_info_ map[int32]*ProtoMsg.PbSvrBaseInfo
	svr_type_ ProtoMsg.EmSvrType
	publish_svrs_ []*common_session.ClientSession
}


type SVR_TYPE_INFO map[ProtoMsg.EmSvrType]*svrInfo
type SVR_GROUP_INFO map[int32]SVR_TYPE_INFO


func genSvrKey(group_id, svr_id int32, svr_type ProtoMsg.EmSvrType) string {
	return fmt.Sprintf("%d_%d_%v", group_id, svr_id, svr_type)
}


type svrsMgr struct {
	m_svrs_info_ SVR_GROUP_INFO
	m_publish_info_ map[string][]ProtoMsg.EmSvrType
}


func(mgr *svrsMgr) Start() (err error) {
	//注册消息
	err = global.ClusterSvrGlobal.GetMsgDispatcher().Register(uint32(ProtoMsg.EmSSMsgId_SVR_MSG_REGISTER_CLUSTER),
		&ProtoMsg.PbSvrRegisterClusterReqMsg{}, mgr.onrecv_register)
	if err != nil {
		return
	}

	return
}

func(mgr *svrsMgr) Stop() {

}

func (mgr *svrsMgr) onrecv_register(sink interface{}, h common.IMsgHead, msg interface{}) {
	cli_session := sink.(*common_session.ClientSession)

	head := h.(*common.ProtocolInnerHead)
	reg_msg := msg.(*ProtoMsg.PbSvrRegisterClusterReqMsg)
	if reg_msg.SvrInfo == nil {
		log.Error("onrecv_register svr info nil")
		return
	}

	group_info, ok := mgr.m_svrs_info_[reg_msg.SvrInfo.GroupId]
	if !ok {
		mgr.m_svrs_info_[reg_msg.SvrInfo.GroupId] = make(SVR_TYPE_INFO)
		group_info = mgr.m_svrs_info_[reg_msg.SvrInfo.GroupId]
	}

	type_info, ok := group_info[reg_msg.SvrInfo.SvrType]
	if !ok {
		group_info[reg_msg.SvrInfo.SvrType] = &svrInfo{
			svr_type_: reg_msg.SvrInfo.SvrType,
			svrs_info_: make(map[int32]*ProtoMsg.PbSvrBaseInfo),
			publish_svrs_ : []*common_session.ClientSession{},
		}
		type_info = group_info[reg_msg.SvrInfo.SvrType]
	}
	type_info.svrs_info_[reg_msg.SvrInfo.SvrId] = reg_msg.SvrInfo

	//广播到关注此服务的服务
	if len(type_info.publish_svrs_) > 0 {
		b_head := &common.ProtocolInnerHead{
			Msg_id_: uint32(ProtoMsg.EmSSMsgId_SVR_MSG_BROAD_CLUSTER),
		}
		b_body := &ProtoMsg.PbSvrBroadClusterMsg{
			OprType: ProtoMsg.EmClusterOprType_Add,
			SvrInfo: reg_msg.SvrInfo,
		}
		for _, svr := range type_info.publish_svrs_ {
			svr.Send(b_head, b_body)
		}
	}

	//生成订阅
	resp_msg := &ProtoMsg.PbSvrRegisterClusterResMsg{
		Ret: &ProtoMsg.Ret{
			ErrCode: 0,
		},
		Svrs: []*ProtoMsg.PbSvrBaseInfo{},
	}

	publish_key := genSvrKey(reg_msg.SvrInfo.GroupId, reg_msg.SvrInfo.SvrId, reg_msg.SvrInfo.SvrType)
	mgr.m_publish_info_[publish_key] = reg_msg.SvrTypes
	for _, publish_srv_type := range reg_msg.SvrTypes {
		p_type_info, ok := group_info[publish_srv_type]
		if !ok {
			group_info[publish_srv_type] = &svrInfo{
				svr_type_: publish_srv_type,
				svrs_info_: make(map[int32]*ProtoMsg.PbSvrBaseInfo),
				publish_svrs_ : []*common_session.ClientSession{ cli_session },
			}
		} else {
			p_type_info.publish_svrs_ = append(p_type_info.publish_svrs_, cli_session)
			for _, svr_info := range p_type_info.svrs_info_ {
				resp_msg.Svrs = append(resp_msg.Svrs, svr_info)
			}
		}
	}

	cli_session.SetAttribute(global.Session_attribute_key_Svr_info, reg_msg.SvrInfo)
	cli_session.Send(head, resp_msg)

	log.Release("svrsMg::onrecv_register reg svr:%+v, publish types:%+v", reg_msg.SvrInfo, reg_msg.SvrTypes)
}

func (mgr *svrsMgr) RemoveSvr(session *common_session.ClientSession) {
	i_svr_info := session.GetAttribute(global.Session_attribute_key_Svr_info)
	if i_svr_info == nil {
		return
	}
	svr_info := i_svr_info.(*ProtoMsg.PbSvrBaseInfo)

	publish_key := genSvrKey(svr_info.GroupId, svr_info.SvrId, svr_info.SvrType)
	publish_info, ok := mgr.m_publish_info_[publish_key]
	if !ok {
		return
	}

	//
	//取消订阅
	if len(publish_info) > 0 {
		for _, publish_type := range publish_info {
			group_info, ok := mgr.m_svrs_info_[svr_info.GroupId]
			if !ok {
				continue
			}

			type_info, ok := group_info[publish_type]
			if !ok {
				continue
			}

			for i, s := range type_info.publish_svrs_ {
				if session ==  s {
					type_info.publish_svrs_ = append(type_info.publish_svrs_, type_info.publish_svrs_[i+1:]...)
					break
				}
			}
		}
	}
	delete(mgr.m_publish_info_, publish_key)

	//取消被订阅
	group_info, ok := mgr.m_svrs_info_[svr_info.GroupId]
	if !ok {
		return
	}

	type_info, ok := group_info[svr_info.SvrType]
	if !ok {
		return
	}

	delete(type_info.svrs_info_, svr_info.SvrId)
	if len(type_info.publish_svrs_) > 0 {
		b_head := &common.ProtocolInnerHead{
			Msg_id_: uint32(ProtoMsg.EmSSMsgId_SVR_MSG_BROAD_CLUSTER),
		}
		b_body := &ProtoMsg.PbSvrBroadClusterMsg{
			OprType: ProtoMsg.EmClusterOprType_Del,
			SvrInfo: svr_info,
		}

		for _, csession := range type_info.publish_svrs_ {
			csession.Send(b_head, b_body)
		}
	}

	log.Release("svrsMgr::RemoveSvr reg svr:%+v", svr_info)
}