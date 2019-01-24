package net

import (
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/apps/common/global"
	"github.com/SpiritDyn123/gocygame/apps/common/net/session"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/golang/protobuf/proto"
	"github.com/name5566/leaf/log"
)


type ClusterSession struct {
	*session.SvrSession
}

func (ssession *ClusterSession)OnCreate()  {
	ssession.SvrSession.OnCreate()

	svr_publish, ok := ssession.Svr_global_.(global.IServerGlobal_Publish)
	if !ok || svr_publish.GetPublishSvrs() == nil || svr_publish.GetSvrBaseInfo() == nil {
		log.Error("ClusterSession Svr_global_ is not publish svr")
		return
	}

	//发送注册和订阅消息
	head := &common.ProtocolInnerHead{
		Msg_id_: uint32(ProtoMsg.EmMsgId_SVR_MSG_REGISTER_CLUSTER),
	}

	reg_msg := &ProtoMsg.PbSvrRegisterClusterReqMsg{
		SvrInfo: svr_publish.GetSvrBaseInfo(),
		SvrTypes: svr_publish.GetPublishSvrs(),
	}

	ssession.Send(head, reg_msg)
}

func (ssession *ClusterSession) String() string {
	return fmt.Sprintf("ClusterSession:{id:%d, info:{%+v}, last_check_time_:%v}", ssession.Id(),
		ssession.Config_info_, ssession.Last_check_time_.Format("2006-01-02 15:04:05"))
}

func CreateSvrSession(tcp_session *tcp.Session, svr_global global.IServerGlobal, config_info *ProtoMsg.PbSvrBaseInfo) global.ILogicSession {
	return &ClusterSession{
		SvrSession: session.CreateSvrSession(tcp_session, svr_global, config_info).(*session.SvrSession),
	}
}


type TcpClientCluster struct {
	TcpClientMgr

	Svr_info_ *common.Cfg_Json_Svr_Item //集群服务器地址
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

	cluster.M_create_session_cb_[ProtoMsg.EmSvrType_Cluster] = CreateSvrSession
	err = cluster.AddClient(&ProtoMsg.PbSvrBaseInfo{
		GroupId: int32(0),
		SvrId: int32(cluster.Svr_info_.Id_),
		SvrType: ProtoMsg.EmSvrType_Cluster,
		Addr: cluster.Svr_info_.Addr_,
		Ttl: int32(cluster.Svr_info_.Ttl_), //用系统监听的ttl
		Timeout: int32(cluster.Svr_info_.Timeout_),
	},)
	if err != nil {
		return
	}

	return
}

func (cluster *TcpClientCluster) Stop() {
	cluster.TcpClientMgr.Stop()
}

func(cluster *TcpClientCluster) onRecvRegisterSvr(sink interface{}, head common.IMsgHead, msg proto.Message) {
	_ = sink.(*ClusterSession)
	_ = head.(*common.ProtocolInnerHead)

	resp_msg := msg.(*ProtoMsg.PbSvrRegisterClusterResMsg)
	for _, svr_info := range resp_msg.Svrs {
		cluster.AddClient(svr_info)
		log.Release("TcpClientCluster::onRecvRegisterSvr svr:%+v", svr_info)
	}
}

func(cluster *TcpClientCluster) onRecvBroadSvr(sink interface{}, head common.IMsgHead, msg proto.Message) {
	_ = sink.(*ClusterSession)
	_ = head.(*common.ProtocolInnerHead)

	resp_msg := msg.(*ProtoMsg.PbSvrBroadClusterMsg)
	if resp_msg.OprType == ProtoMsg.EmClusterOprType_Add {
		cluster.AddClient( resp_msg.SvrInfo)
	} else if resp_msg.OprType == ProtoMsg.EmClusterOprType_Del {
		cluster.RemoveClient( resp_msg.SvrInfo)
	}

	log.Release("TcpClientCluster::onRecvRegisterSvr opr_type:%v, svr:%+v", resp_msg.OprType, resp_msg.SvrInfo)
}