package global


type IDbOperationMgr interface {
	Start() error
	Stop()
}