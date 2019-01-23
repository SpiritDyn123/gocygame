package global

type ISvrsMgr interface {
	Start() error
	Stop()
}