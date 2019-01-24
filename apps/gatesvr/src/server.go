package src

import (
	"github.com/SpiritDyn123/gocygame/apps/common/net"
	"github.com/SpiritDyn123/gocygame/apps/common/net/codec"
	"github.com/SpiritDyn123/gocygame/apps/common/tools"
	"github.com/SpiritDyn123/gocygame/apps/gatesvr/src/cluster"
	"github.com/SpiritDyn123/gocygame/apps/gatesvr/src/global"
	"github.com/SpiritDyn123/gocygame/libs/utils"
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/gatesvr/src/etc"
	"github.com/SpiritDyn123/gocygame/libs/chanrpc"
	"github.com/SpiritDyn123/gocygame/libs/go"
	"github.com/SpiritDyn123/gocygame/libs/timer"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/libs/log"
)

type GateSvrGlobal struct {
	utils.Pooller
	net_ser tcp.INetServer

	wheel_timer_ timer.WheelTimer

	msg_dispatcher_ tools.IMsgDispatcher

	cluster_mgr_ global.IClusterMgr
}

func (svr *GateSvrGlobal) GetName() string {
	return fmt.Sprintf("%s_%d_%d", etc.Gate_Config.System_.Svr_name_,
		etc.Gate_Config.System_.Svr_group_id_, etc.Gate_Config.System_.Svr_id_)
}

func (svr *GateSvrGlobal) Start() (err error) {
	global.GateSvrGlobal = svr

	svr.ChanServer = chanrpc.NewServer(etc.Chan_Server_Len)
	svr.ChanServer.Register(common.Chanrpc_key_tcp_accept, svr.onTcpAccept)
	svr.ChanServer.Register(common.Chanrpc_key_tcp_recv, svr.onTcpRecv)
	svr.ChanServer.Register(common.Chanrpc_key_tcp_close, svr.onTcpClose)

	svr.GoServer = g.New(common.Default_Go_Server_Len)

	//定时器
	svr.wheel_timer_ = timer.CreateWheelTimer()
	svr.TimerServer = timer.NewDispatcher(10)
	svr.TimerServer.AfterFunc(common.Default_Svr_Logic_time, svr.onTimer)

	//消息管理器
	svr.msg_dispatcher_ = tools.CreateMsgDispatcher()

	//集群管理器
	svr.cluster_mgr_ = cluster.ClusterMgr
	if err = svr.cluster_mgr_.Start(); err != nil {
		return
	}

	//启动socket
	protocol := &codec.ProtoClientProtocol{
		Endian_: common.Default_Net_Endian,
	}

	svr.net_ser, err = net.CreateTcpServer(&etc.Gate_Config.System_, etc.Gate_Config.Tls_, protocol, common.Default_Net_Head_Len,
		common.Default_Net_Endian, common.Default_Send_Chan_Len, svr.ChanServer)
	if err != nil {
		return
	}
	if !svr.net_ser.Start() {
		return fmt.Errorf("netserver start error")
	}

	return nil
}

func (svr *GateSvrGlobal) Close() {
	svr.net_ser.Stop()
}

func (svr *GateSvrGlobal) Pool(cs chan bool) {
	svr.Pooller.Pool(cs)
}

func (svr *GateSvrGlobal) GetPriority() int {
	return 0
}

func (svr *GateSvrGlobal) GetWheelTimer() timer.WheelTimer {
	return svr.wheel_timer_
}

func (svr *GateSvrGlobal) GetMsgDispatcher() tools.IMsgDispatcher {
	return svr.msg_dispatcher_
}

func (svr *GateSvrGlobal) onTimer() {
	svr.wheel_timer_.Step()
	svr.TimerServer.AfterFunc(common.Default_Svr_Logic_time, svr.onTimer)
}

func (svr *GateSvrGlobal) onTcpAccept(args []interface{}) {
	session := args[0].(*tcp.Session)
	log.Release("onTcpAccept session:%v", session)
}

func (svr *GateSvrGlobal) onTcpRecv(args []interface{}) {
	session := args[0].(*tcp.Session)
	log.Release("onTcpRecv session:%v recv:%v", session, args[1:])
	session.Send(args[1].([]interface{})...)
}

func (svr *GateSvrGlobal) onTcpClose(args []interface{}) {
	session := args[0].(*tcp.Session)
	log.Release("onTcpClose session:%v", session)
}