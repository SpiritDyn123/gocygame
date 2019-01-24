package global

import (
	"github.com/SpiritDyn123/gocygame/apps/common/tools"
	"github.com/SpiritDyn123/gocygame/libs/utils"
	"github.com/SpiritDyn123/gocygame/libs/timer"
)

type IGameSvrGlobal interface {
	utils.IPooller
	GetMsgDispatcher() tools.IMsgDispatcher
	GetWheelTimer() timer.WheelTimer
}

var GameSvrGlobal IGameSvrGlobal