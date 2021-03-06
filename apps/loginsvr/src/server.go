package src

import (
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/apps/common/net"
	"github.com/SpiritDyn123/gocygame/apps/common/net/codec"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/apps/common/tools"
	"github.com/SpiritDyn123/gocygame/apps/loginsvr/src/etc"
	"github.com/SpiritDyn123/gocygame/apps/loginsvr/src/global"
	"github.com/SpiritDyn123/gocygame/libs/chanrpc"
	"github.com/SpiritDyn123/gocygame/libs/go"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/libs/timer"
	"github.com/SpiritDyn123/gocygame/libs/utils"
	"github.com/SpiritDyn123/gocygame/apps/loginsvr/src/login"
)


type LoginSvrGlobal struct {
	utils.Pooller
	net_ser tcp.INetServer

	wheel_timer_ timer.WheelTimer

	msg_dispatcher_ tools.IMsgDispatcher

	svrs_mgr_ *net.SvrsMgr
	login_mgr_ global.ILoginMgr
}

func (svr *LoginSvrGlobal) GetName() string {
	return fmt.Sprintf("%s_%d_%d", etc.Login_Config.System_.Svr_name_,
		etc.Login_Config.System_.Svr_group_id_, etc.Login_Config.System_.Svr_id_)
}

func (svr *LoginSvrGlobal) Start() (err error) {

	global.LoginSvrGlobal = svr

	svr.ChanServer = chanrpc.NewServer(common.Default_Chan_Server_Len)
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

	//登陆器
	svr.login_mgr_ = login.LoginMgr
	if err = svr.login_mgr_.Start(); err != nil {
		return
	}

	//服务管理器
	svr.svrs_mgr_ = &net.SvrsMgr{
		Svr_global_: svr,
		Client_msg_dispatcher_: svr.GetMsgDispatcher(),
		Svr_msg_dispatcher_: svr.GetMsgDispatcher(),
		Publish_svrs_: []ProtoMsg.EmSvrType{
			ProtoMsg.EmSvrType_DB,
		},
		Cluster_svr_info_: &etc.Login_Config.Cluster_,
	}

	if err = svr.svrs_mgr_.Start(); err != nil {
		return
	}

	//启动socket
	protocol := &codec.ProtoInnerProtocol{
		Endian_: common.Default_Net_Endian,
	}

	svr.net_ser, err = net.CreateTcpServer(&etc.Login_Config.System_, nil, protocol, common.Default_Net_Head_Len,
		common.Default_Net_Endian, common.Default_Send_Chan_Len, svr.ChanServer)
	if err != nil {
		return
	}
	if !svr.net_ser.Start() {
		return fmt.Errorf("netserver start error")
	}

	return nil
}

func (svr *LoginSvrGlobal) Close() {
	svr.net_ser.Stop()
	svr.svrs_mgr_.Stop()
	svr.login_mgr_.Stop()
}

func (svr *LoginSvrGlobal) Pool(cs chan bool) {
	svr.Pooller.Pool(cs)
}

func (svr *LoginSvrGlobal) GetPriority() int {
	return 0
}

func (svr *LoginSvrGlobal) GetMsgDispatcher() tools.IMsgDispatcher {
	return svr.msg_dispatcher_
}

func (svr *LoginSvrGlobal) GetWheelTimer() timer.WheelTimer {
	return svr.wheel_timer_
}

func (svr *LoginSvrGlobal) GetSvrsMgr() *net.SvrsMgr {
	return svr.svrs_mgr_
}

func (svr *LoginSvrGlobal) GetSvrBaseInfo() *ProtoMsg.PbSvrBaseInfo{
	return &ProtoMsg.PbSvrBaseInfo{
		GroupId: int32(etc.Login_Config.System_.Svr_group_id_),
		SvrId: int32(etc.Login_Config.System_.Svr_id_),
		SvrType: ProtoMsg.EmSvrType_Login,
		Addr: etc.Login_Config.System_.Svr_addr_,
		Ttl: int32(etc.Login_Config.System_.Svr_ttl_), //用系统监听的ttl
		Timeout: int32(etc.Login_Config.System_.Svr_timeout_),
	}
}

func (svr *LoginSvrGlobal)	GetLoginMgr() global.ILoginMgr {
	return svr.login_mgr_
}

func (svr *LoginSvrGlobal) onTimer() {
	svr.wheel_timer_.Step()
	svr.TimerServer.AfterFunc(common.Default_Svr_Logic_time, svr.onTimer)
}

func (svr *LoginSvrGlobal) onTcpAccept(args []interface{}) {
	svr.svrs_mgr_.OnAccept(args[0].(*tcp.Session))
}

func (svr *LoginSvrGlobal) onTcpRecv(args []interface{}) {
	svr.svrs_mgr_.OnRecv(args[0].(*tcp.Session), args[1])
}

func (svr *LoginSvrGlobal) onTcpClose(args []interface{}) {
	svr.svrs_mgr_.OnClose(args[0].(*tcp.Session))
}