package global


type IPlayerMgr interface {
	Start() error
	Stop()
}