package global

import (
	"github.com/SpiritDyn123/gocygame/apps/common/global"
)

type IGateSvrGlobal interface {
	global.IServerGlobal
}

var GateSvrGlobal IGateSvrGlobal