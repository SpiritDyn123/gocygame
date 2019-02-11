package global

import (
	"github.com/SpiritDyn123/gocygame/apps/common/global"
	"github.com/SpiritDyn123/gocygame/apps/common/net"
	"github.com/SpiritDyn123/gocygame/apps/common/tools"
)

type IGateSvrGlobal interface {
	global.IServerGlobal
	GetSvrsMgr() *net.SvrsMgr
	GetClientMsgParser() tools.IMsgDispatcher
	GetSvrMsgParser() tools.IMsgDispatcher
	GetPlayerMgr() IPlayerMgr

}

var GateSvrGlobal IGateSvrGlobal