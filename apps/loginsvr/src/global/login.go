package global

type ILoginMgr interface {
	Start() (err error)
	Stop()
}

