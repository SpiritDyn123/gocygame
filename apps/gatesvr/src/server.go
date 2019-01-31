package src

import (
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/apps/common/net"
	"github.com/SpiritDyn123/gocygame/apps/common/net/codec"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/apps/common/tools"
	"github.com/SpiritDyn123/gocygame/apps/gatesvr/src/etc"
	"github.com/SpiritDyn123/gocygame/apps/gatesvr/src/global"

	"github.com/SpiritDyn123/gocygame/libs/chanrpc"
	"github.com/SpiritDyn123/gocygame/libs/go"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/libs/timer"
	"github.com/SpiritDyn123/gocygame/libs/utils"
	"github.com/SpiritDyn123/gocygame/apps/gatesvr/src/player"
)

type GateSvrGlobal struct {
	utils.Pooller
	net_ser tcp.INetServer

	wheel_timer_ timer.WheelTimer

	msg_dispatcher_ tools.IMsgDispatcher

	svrs_mgr_ *net.SvrsMgr

	player_mgr_ global.IPlayerMgr
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
	svr.msg_dispatcher_ = tools.CreateMsgDispatcherWithTransmit(svr.onRecvTransmit)


	//玩家管理器
	svr.player_mgr_ = player.PlayerMgr
	if err = svr.player_mgr_.Start(); err != nil {
		return
	}

	//服务管理器
	svr.svrs_mgr_ = &net.SvrsMgr{
		Svr_global_: svr,
		Publish_svrs_: []ProtoMsg.EmSvrType{
			ProtoMsg.EmSvrType_Login,
			ProtoMsg.EmSvrType_Gs,
		},
		Cluster_svr_info_: &etc.Gate_Config.Cluster_,
	}

	if err = svr.svrs_mgr_.Start(); err != nil {
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
	svr.svrs_mgr_.Stop()
	svr.player_mgr_.Stop()
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

func(svr *GateSvrGlobal) GetPublishSvrs() []ProtoMsg.EmSvrType {
	return  []ProtoMsg.EmSvrType{
		ProtoMsg.EmSvrType_Gs,
		ProtoMsg.EmSvrType_Login,
	}
}

func (svr *GateSvrGlobal) GetSvrBaseInfo() *ProtoMsg.PbSvrBaseInfo{
	return &ProtoMsg.PbSvrBaseInfo{
		GroupId: int32(etc.Gate_Config.System_.Svr_group_id_),
		SvrId: int32(etc.Gate_Config.System_.Svr_id_),
		SvrType: ProtoMsg.EmSvrType_Gate,
		Addr: etc.Gate_Config.System_.Svr_addr_,
		Ttl: int32(etc.Gate_Config.System_.Svr_ttl_), //用系统监听的ttl
		Timeout: int32(etc.Gate_Config.System_.Svr_timeout_),
	}
}
func (svr *GateSvrGlobal) onTimer() {
	svr.wheel_timer_.Step()
	svr.TimerServer.AfterFunc(common.Default_Svr_Logic_time, svr.onTimer)
}

func (svr *GateSvrGlobal) onTcpAccept(args []interface{}) {
	session := args[0].(*tcp.Session)
	svr.player_mgr_.OnAccept(session)
}

func (svr *GateSvrGlobal) onTcpRecv(args []interface{}) {
	session := args[0].(*tcp.Session)
	svr.player_mgr_.OnRecv(session, args[1])
}

func (svr *GateSvrGlobal) onTcpClose(args []interface{}) {
	session := args[0].(*tcp.Session)
	svr.player_mgr_.OnClose(session)
}

func (svr *GateSvrGlobal) onRecvTransmit(sink interface{}, head common.IMsgHead, msg []byte) {

}