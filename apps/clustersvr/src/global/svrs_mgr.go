package global

import (
	"github.com/SpiritDyn123/gocygame/apps/common/net/session"
)

const (
	Session_attribute_key_Svr_info = "attribute_key_svr_info"
)
type ISvrsMgr interface {
	Start() error
	Stop()
	RemoveSvr(session *session.ClientSession)
}