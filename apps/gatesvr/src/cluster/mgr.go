package cluster

import (
	"encoding/binary"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/apps/gatesvr/src/session"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/apps/common/net/codec"
	"github.com/SpiritDyn123/gocygame/apps/gatesvr/src/etc"
	"github.com/SpiritDyn123/gocygame/apps/gatesvr/src/global"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/golang/protobuf/proto"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"time"
)

var ClusterMgr global.IClusterMgr
func init() {
	ClusterMgr = &clusterMgr{}
}

type clusterMgr struct {
	cluster_cli_ *tcp.Client
}

func(mgr *clusterMgr) Start() (err error) {

	//注册消息处理
	global.GateSvrGlobal.GetMsgDispatcher().Register(ProtoMsg.EmMsgId_SVR_MSG_REGISTER_CLUSTER, &ProtoMsg.PbSvrRegisterClusterResMsg{},
		mgr.onRecvRegisterSvr)
	global.GateSvrGlobal.GetMsgDispatcher().Register(ProtoMsg.EmMsgId_SVR_MSG_BROAD_CLUSTER, &ProtoMsg.PbSvrBroadClusterMsg{},
		mgr.onRecvBroadSvr)

	//连接集群管理器
	chan_server := global.GateSvrGlobal.GetChanServer()
	chan_server.Register(common.Chanrpc_key_tcp_inner_accept, mgr.onTcpConnect)
	chan_server.Register(common.Chanrpc_key_tcp_inner_close, mgr.onTcpClose)
	chan_server.Register(common.Chanrpc_key_tcp_inner_recv, mgr.onTcpRecv)

	msg_parser := tcp.NewMsgParser()
	msg_parser.SetIncludeHead(true)
	msg_parser.SetByteOrder(common.Default_Net_Endian == binary.LittleEndian)
	msg_parser.SetMsgLen(common.Default_Net_Head_Len, uint32(etc.Gate_Config.System_.Svr_in_bytes_), uint32(etc.Gate_Config.System_.Svr_in_bytes_))
	mgr.cluster_cli_ = &tcp.Client{
		AutoReconnect: true,
		ConnectInterval:3 * time.Second,

		Network: "tcp",
		Addr: etc.Gate_Config.Cluster_.Addr_,
		DialTimeOut: 3 *time.Second,
		ConnectNum: 1,
		Protocol: &codec.ProtoInnerProtocol{Endian_: common.Default_Net_Endian},
		MsgParser: msg_parser,
		SendChanSize: common.Default_Svr_Send_Chan_Len,

		ChanServer: global.GateSvrGlobal.GetChanServer(),
		ConnectKey: common.Chanrpc_key_tcp_inner_accept,
		ClosedKey: common.Chanrpc_key_tcp_inner_close,
		RecvKey: common.Chanrpc_key_tcp_inner_recv,

		Data: &ProtoMsg.PbSvrBaseInfo{
			GroupId: int32(etc.Gate_Config.System_.Svr_group_id_),
			SvrId: int32(etc.Gate_Config.Cluster_.Id_),
			SvrType: ProtoMsg.EmSvrType_Cluster,
			Addr: etc.Gate_Config.Cluster_.Addr_,
			Ttl: int32(etc.Gate_Config.Cluster_.Ttl_),
			Timeout: int32(etc.Gate_Config.Cluster_.Timeout_),
		},
	}

	mgr.cluster_cli_.Connect()

	return
}

func(mgr *clusterMgr) Stop() {
	mgr.cluster_cli_.Stop()
}

func (mgr *clusterMgr) onTcpConnect(args []interface{}) {
	tcp_session := args[0].(*tcp.Session)

	logic_session := session.CreateSvrSession(tcp_session, tcp_session.Data().(*ProtoMsg.PbSvrBaseInfo))
	logic_session.OnCreate()
}

func (mgr *clusterMgr) onTcpClose(args []interface{}) {
	//tcp_session := args[0].(*tcp.Session)
	log.Release("")
}

func (mgr *clusterMgr) onTcpRecv(args []interface{}) {

}

func (mgr *clusterMgr) onRecvRegisterSvr(sink interface{}, head common.IMsgHead, msg proto.Message) {

}

func (mgr *clusterMgr) onRecvBroadSvr(sink interface{}, head common.IMsgHead, msg proto.Message) {

}
