package global

import (
	"github.com/SpiritDyn123/gocygame/apps/common/tools"
	"github.com/SpiritDyn123/gocygame/libs/timer"
	"github.com/SpiritDyn123/gocygame/libs/utils"
)

type IGateSvrGlobal interface {
	utils.IPooller
	GetWheelTimer() timer.WheelTimer
	GetMsgDispatcher() tools.IMsgDispatcher
}

var GateSvrGlobal IGateSvrGlobal