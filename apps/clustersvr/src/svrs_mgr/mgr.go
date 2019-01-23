package svrs_mgr

import (
	"github.com/SpiritDyn123/gocygame/apps/clustersvr/src/global"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/golang/protobuf/proto"
	"libs/log"
	"libs/net/tcp"
)

var SvrsMgr global.ISvrsMgr
func init() {
	SvrsMgr = &svrsMgr{
		m_svrs_info_: make(SVR_GROUP_INFO),
	}
}

type svrInfo  struct {
	svrs_info_ map[int32]*ProtoMsg.PbSvrBaseInfo
	svr_type_ ProtoMsg.EmSvrType
	publish_svrs_ *tcp.Session
}


type SVR_TYPE_INFO map[ProtoMsg.EmSvrType]*svrInfo
type SVR_GROUP_INFO map[int32]SVR_TYPE_INFO

type svrsMgr struct {
	m_svrs_info_ SVR_GROUP_INFO
}

func(mgr *svrsMgr) Start() (err error) {
	//注册消息
	err = global.ClusterSvrGlobal.GetMsgDispatcher().Register(ProtoMsg.EmMsgId_SVR_MSG_REGISTER_CLUSTER,
		&ProtoMsg.PbSvrRegisterClusterReqMsg{}, mgr.onrecv_register)
	if err != nil {
		return
	}

	return
}

func(mgr *svrsMgr) Stop() {

}

func (mgr *svrsMgr) onrecv_register(sink interface{}, h common.IMsgHead, msg proto.Message) {
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
		}
		type_info = group_info[reg_msg.SvrInfo.SvrType]
	}

	type_info.svrs_info_[reg_msg.SvrInfo.SvrId] = reg_msg.SvrInfo

	resp_msg := ProtoMsg.PbSvrRegisterClusterResMsg{
		Ret: &ProtoMsg.Ret{
			ErrCode: 0,
		},
	}

	sink.(*tcp.Session).Send(head, resp_msg)
}