package session

import (
	"time"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/SpiritDyn123/gocygame/libs/timer"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
)

type BaseSession struct {
	*tcp.Session
	Last_check_time_ time.Time

	wheel_timer_ 	timer.WheelTimer
	wtId_ uint64	//心跳定时器 id
	closed_ bool

	Config_info_ *ProtoMsg.PbSvrBaseInfo
}
