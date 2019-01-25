package svrs_mgr

import "github.com/SpiritDyn123/gocygame/apps/common/net/session"

type svrInfo struct {
	session_ *session.SvrSession
}

func(svr *svrInfo) onClose() {

	svr.session_.Close()
	svr.session_ = nil
}
