package global

import (
	"github.com/SpiritDyn123/gocygame/apps/common/net/session"
)

type ISvrsMgr interface {
	Start() (err error)
	Stop()
	OnSvrClose(sssesion *session.SvrSession)
}