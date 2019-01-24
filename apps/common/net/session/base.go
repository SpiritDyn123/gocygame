package session

import (
	"github.com/SpiritDyn123/gocygame/apps/common/global"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"time"
)

type BaseSession struct {
	*tcp.Session
	Last_check_time_ time.Time

	Svr_global_  global.IServerGlobal
	wtId_ uint64	//心跳定时器 id
	closed_ bool

	Config_info_ *ProtoMsg.PbSvrBaseInfo
}
