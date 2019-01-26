package net

import (
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/apps/common/global"
	"github.com/SpiritDyn123/gocygame/apps/common/net/session"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/golang/protobuf/proto"
	"github.com/name5566/leaf/log"
)

type TcpClientCluster struct {
	TcpClientMgr

	Cluster_svr_info_ *common.Cfg_Json_Svr_Item //集群服务器地址
	Svr_info_ *ProtoMsg.PbSvrBaseInfo 			//服务器信息
	Publish_svrs_ []ProtoMsg.EmSvrType
}

func (cluster *TcpClientCluster) Start() (err error){
	err = cluster.TcpClientMgr.Start()
	if err != nil {
		return
	}

	//注册消息处理
	cluster.Svr_global_.GetMsgDispatcher().Register(ProtoMsg.EmMsgId_SVR_MSG_REGISTER_CLUSTER, &ProtoMsg.PbSvrRegisterClusterResMsg{},
		cluster.onRecvRegisterSvr)
	cluster.Svr_global_.GetMsgDispatcher().Register(ProtoMsg.EmMsgId_SVR_MSG_BROAD_CLUSTER, &ProtoMsg.PbSvrBroadClusterMsg{},
		cluster.onRecvBroadSvr)

	cluster.M_create_session_cb_[ProtoMsg.EmSvrType_Cluster] = cluster.createClusterSession
	err = cluster.AddClient(&ProtoMsg.PbSvrBaseInfo{
		GroupId: int32(0),
		SvrId: int32(cluster.Cluster_svr_info_.Id_),
		SvrType: ProtoMsg.EmSvrType_Cluster,
		Addr: cluster.Cluster_svr_info_.Addr_,
		Ttl: int32(cluster.Cluster_svr_info_.Ttl_), //用系统监听的ttl
		Timeout: int32(cluster.Cluster_svr_info_.Timeout_),
	},)
	if err != nil {
		return
	}

	return
}

func (cluster *TcpClientCluster) Stop() {
	cluster.TcpClientMgr.Stop()
}

func (cluster *TcpClientCluster) createClusterSession(tcp_session *tcp.Session, svr_global global.IServerGlobal,
	config_info *ProtoMsg.PbSvrBaseInfo) global.ILogicSession {
	cluster_session := session.CreateSvrSession(tcp_session, svr_global, config_info).(*session.SvrSession)

	//连接成功的回调里需要注册服务器信息
	cluster_session.SetSessionEventCB(session.SessionEvent_Accept, func(logic_session global.ILogicSession) {
		//发送注册和订阅消息
		head := &common.ProtocolInnerHead{
			Msg_id_: uint32(ProtoMsg.EmMsgId_SVR_MSG_REGISTER_CLUSTER),
		}

		reg_msg := &ProtoMsg.PbSvrRegisterClusterReqMsg{
			SvrInfo: cluster.Svr_info_,
			SvrTypes: cluster.Publish_svrs_,
		}

		logic_session.Send(head, reg_msg)
	})

	return cluster_session
}

func(cluster *TcpClientCluster) onRecvRegisterSvr(sink interface{}, head common.IMsgHead, msg proto.Message) {
	_ = sink.(*session.SvrSession)
	_ = head.(*common.ProtocolInnerHead)

	resp_msg := msg.(*ProtoMsg.PbSvrRegisterClusterResMsg)
	for _, svr_info := range resp_msg.Svrs {
		cluster.AddClient(svr_info)
		log.Release("TcpClientCluster::onRecvRegisterSvr svr:%+v", svr_info)
	}
}

func(cluster *TcpClientCluster) onRecvBroadSvr(sink interface{}, head common.IMsgHead, msg proto.Message) {
	_ = sink.(*session.SvrSession)
	_ = head.(*common.ProtocolInnerHead)

	resp_msg := msg.(*ProtoMsg.PbSvrBroadClusterMsg)
	//不能连接自己的服务
	if resp_msg.SvrInfo.SvrType == cluster.Svr_info_.SvrType &&
		resp_msg.SvrInfo.GroupId == cluster.Svr_info_.GroupId &&
		resp_msg.SvrInfo.SvrId == cluster.Svr_info_.SvrId {
		return
	}

	if resp_msg.OprType == ProtoMsg.EmClusterOprType_Add {
		cluster.AddClient( resp_msg.SvrInfo)
	} else if resp_msg.OprType == ProtoMsg.EmClusterOprType_Del {
		cluster.RemoveClient( resp_msg.SvrInfo)
	}

	log.Release("TcpClientCluster::onRecvRegisterSvr opr_type:%v, svr:%+v", resp_msg.OprType, resp_msg.SvrInfo)
}