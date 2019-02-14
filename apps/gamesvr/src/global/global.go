package global

import (
	"github.com/SpiritDyn123/gocygame/apps/common/net"
	"github.com/SpiritDyn123/gocygame/apps/common/tools"
	"github.com/SpiritDyn123/gocygame/libs/timer"
	"github.com/SpiritDyn123/gocygame/libs/utils"
)

type IGameSvrGlobal interface {
	utils.IPooller
	GetMsgDispatcher() tools.IMsgDispatcher
	GetWheelTimer() timer.WheelTimer

	GetSvrsMgr() *net.SvrsMgr
	GetPlayerMgr() IPlayerMgr
}

var GameSvrGlobal IGameSvrGlobal