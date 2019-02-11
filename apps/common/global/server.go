package global

import (
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/apps/common/tools"
	"github.com/SpiritDyn123/gocygame/libs/timer"
	"github.com/SpiritDyn123/gocygame/libs/utils"
)

type IServerGlobal interface {
	utils.IPooller
	GetMsgDispatcher() tools.IMsgDispatcher //通用的消息派发器
	GetWheelTimer() timer.WheelTimer
	GetSvrBaseInfo() *ProtoMsg.PbSvrBaseInfo
}
