package global

import "time"

type ILogicSession interface {
	OnRecv(data interface{})(now time.Time, is_hb bool)
	OnClose()
	OnCreate()
}
