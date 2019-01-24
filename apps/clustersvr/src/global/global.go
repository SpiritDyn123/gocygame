package global

import (
	"github.com/SpiritDyn123/gocygame/apps/common/global"
)

type IClusterSvrGlobal interface {
	global.IServerGlobal

	GetSvrsMgr() ISvrsMgr
}

var ClusterSvrGlobal IClusterSvrGlobal