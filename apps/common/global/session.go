package global

import "time"

type ILogicSession interface {
	Id() uint64
	Send(msg ...interface{}) error
	Close()

	OnRecv(data interface{})(now time.Time, is_hb bool)
	OnClose()
	OnCreate()
}
