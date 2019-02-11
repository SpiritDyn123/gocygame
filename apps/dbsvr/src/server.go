package src

import (
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/apps/common/net"
	"github.com/SpiritDyn123/gocygame/apps/common/net/codec"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/apps/common/tools"
	"github.com/SpiritDyn123/gocygame/apps/dbsvr/src/etc"
	"github.com/SpiritDyn123/gocygame/apps/dbsvr/src/global"
	"github.com/SpiritDyn123/gocygame/libs/chanrpc"
	"github.com/SpiritDyn123/gocygame/libs/go"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/libs/timer"
	"github.com/SpiritDyn123/gocygame/libs/utils"
	"github.com/SpiritDyn123/gocygame/apps/dbsvr/src/db"
	"github.com/SpiritDyn123/gocygame/apps/dbsvr/src/operation"
)


type DBSvrGlobal struct {
	utils.Pooller
	net_ser tcp.INetServer

	wheel_timer_ timer.WheelTimer

	msg_dispatcher_ tools.IMsgDispatcher

	svrs_mgr_ *net.SvrsMgr

	db_mgr_ global.IDBMgr
	db_opr_mgr_ global.IDbOperationMgr
}

func (svr *DBSvrGlobal) GetName() string {
	return fmt.Sprintf("%s_%d_%d", etc.DB_Config.System_.Svr_name_,
		etc.DB_Config.System_.Svr_group_id_, etc.DB_Config.System_.Svr_id_)
}

func (svr *DBSvrGlobal) Start() (err error) {

	global.DBSvrGlobal = svr

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

	//db管理器
	svr.db_mgr_ = db.DBMgr
	if err = svr.db_mgr_.Start(); err != nil {
		return
	}

	//db操作管理器
	svr.db_opr_mgr_ = operation.DbOperationMgr
	if err = svr.db_opr_mgr_.Start(); err != nil {
		return
	}

	//服务管理器
	svr.svrs_mgr_ = &net.SvrsMgr{
		Svr_global_: svr,
		Client_msg_dispatcher_: svr.GetMsgDispatcher(),
		Svr_msg_dispatcher_: svr.GetMsgDispatcher(),
		Publish_svrs_: []ProtoMsg.EmSvrType{},
		Cluster_svr_info_: &etc.DB_Config.Cluster_,
	}

	if err = svr.svrs_mgr_.Start(); err != nil {
		return
	}

	//启动socket
	protocol := &codec.ProtoInnerProtocol{
		Endian_: common.Default_Net_Endian,
	}

	svr.net_ser, err = net.CreateTcpServer(&etc.DB_Config.System_, nil, protocol, common.Default_Net_Head_Len,
		common.Default_Net_Endian, common.Default_Send_Chan_Len, svr.ChanServer)
	if err != nil {
		return
	}
	if !svr.net_ser.Start() {
		return fmt.Errorf("netserver start error")
	}

	return nil
}

func (svr *DBSvrGlobal) Close() {
	svr.net_ser.Stop()
	svr.svrs_mgr_.Stop()
	svr.db_mgr_.Stop()
	svr.db_opr_mgr_.Stop()
}

func (svr *DBSvrGlobal) Pool(cs chan bool) {
	svr.Pooller.Pool(cs)
}

func (svr *DBSvrGlobal) GetPriority() int {
	return 0
}

func (svr *DBSvrGlobal) GetMsgDispatcher() tools.IMsgDispatcher {
	return svr.msg_dispatcher_
}

func (svr *DBSvrGlobal) GetWheelTimer() timer.WheelTimer {
	return svr.wheel_timer_
}

func (svr *DBSvrGlobal) GetSvrsMgr() *net.SvrsMgr {
	return svr.svrs_mgr_
}

func (svr *DBSvrGlobal) GetDBMgr() global.IDBMgr {
	return svr.db_mgr_
}

func (svr *DBSvrGlobal) GetDBOperaitonMgr() global.IDbOperationMgr {
	return svr.db_opr_mgr_
}

func (svr *DBSvrGlobal) GetSvrBaseInfo() *ProtoMsg.PbSvrBaseInfo{
	return &ProtoMsg.PbSvrBaseInfo{
		GroupId: int32(etc.DB_Config.System_.Svr_group_id_),
		SvrId: int32(etc.DB_Config.System_.Svr_id_),
		SvrType: ProtoMsg.EmSvrType_DB,
		Addr: etc.DB_Config.System_.Svr_addr_,
		Ttl: int32(etc.DB_Config.System_.Svr_ttl_), //用系统监听的ttl
		Timeout: int32(etc.DB_Config.System_.Svr_timeout_),
	}
}

func (svr *DBSvrGlobal) onTimer() {
	svr.wheel_timer_.Step()
	svr.TimerServer.AfterFunc(common.Default_Svr_Logic_time, svr.onTimer)
}

func (svr *DBSvrGlobal) onTcpAccept(args []interface{}) {
	svr.svrs_mgr_.OnAccept(args[0].(*tcp.Session))
}

func (svr *DBSvrGlobal) onTcpRecv(args []interface{}) {
	svr.svrs_mgr_.OnRecv(args[0].(*tcp.Session), args[1])
}

func (svr *DBSvrGlobal) onTcpClose(args []interface{}) {
	svr.svrs_mgr_.OnClose(args[0].(*tcp.Session))
}