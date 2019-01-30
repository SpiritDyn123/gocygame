package global

import (
	"github.com/SpiritDyn123/gocygame/apps/common/net"
	"github.com/SpiritDyn123/gocygame/apps/common/tools"
	"github.com/SpiritDyn123/gocygame/libs/utils"
	"github.com/SpiritDyn123/gocygame/libs/timer"
)

type IDBSvrGlobal interface {
	utils.IPooller
	GetMsgDispatcher() tools.IMsgDispatcher
	GetWheelTimer() timer.WheelTimer

	GetSvrsMgr() *net.SvrsMgr

	GetDBMgr() IDBMgr
	GetDBOperaitonMgr() IDbOperationMgr
}

var DBSvrGlobal IDBSvrGlobal