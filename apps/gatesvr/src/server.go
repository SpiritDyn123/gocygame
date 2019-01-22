package src

import (
	"github.com/SpiritDyn123/gocygame/libs/utils"
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/gatesvr/src/etc"
	"github.com/SpiritDyn123/gocygame/libs/chanrpc"
	"github.com/SpiritDyn123/gocygame/libs/go"
	"github.com/SpiritDyn123/gocygame/libs/timer"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
)

type GateSvrGlobal struct {
	utils.Pooller
	net_ser tcp.INetServer
}

func (svr *GateSvrGlobal) GetName() string {
	return fmt.Sprintf("%s_%d_%d", etc.Gate_Config.System_.Svr_name_,
		etc.Gate_Config.System_.Svr_group_id_, etc.Gate_Config.System_.Svr_id_)
}

func (svr *GateSvrGlobal) Start() (err error) {
	svr.ChanServer = chanrpc.NewServer(etc.Chan_Server_Len)
	svr.ChanServer.Register(common.Chanrpc_key_tcp_accept, svr.onTcpAccept)
	svr.ChanServer.Register(common.Chanrpc_key_tcp_recv, svr.onTcpRecv)
	svr.ChanServer.Register(common.Chanrpc_key_tcp_close, svr.onTcpClose)

	svr.GoServer = g.New(etc.Go_Server_Len)
	svr.TimerServer = timer.NewDispatcher(10)

	//启动socket
	if etc.Gate_Config.TLS != nil {

	} else {
		svr.net_ser, err = tcp.CreateTcpServer("tcp", )
	}
	if err != nil {
		return
	}

	return nil
}

func (svr *GateSvrGlobal) Close() {
	panic("implement me")
}

func (svr *GateSvrGlobal) Pool(cs chan bool) {
	panic("implement me")
}

func (svr *GateSvrGlobal) GetPriority() int {
	panic("implement me")
}


func (svr *GateSvrGlobal) onTcpAccept(args []interface{}) {
	panic("implement me")
}

func (svr *GateSvrGlobal) onTcpRecv(args []interface{}) {
	panic("implement me")
}

func (svr *GateSvrGlobal) onTcpClose(args []interface{}) {
	panic("implement me")
}