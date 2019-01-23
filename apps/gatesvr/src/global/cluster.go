package global

type IClusterMgr interface {
	Start() error
	Stop()
}
