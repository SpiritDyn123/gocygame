package tools

type CEvent struct {
	Type    int         //类型
	Obj     interface{} //发送对象
	Content interface{} //发送内容
}

type IEventListener interface {
	OnEvent(event *CEvent) //定时触发
}

type IEventRouter interface {
	AddEventListener(evType int, listener IEventListener) bool
	DelEventListener(evType int, listener IEventListener)
	DoEvent(evType int, obj interface{}, content interface{})
}

type MapListener map[IEventListener]struct{}
type eventRouter struct {
	m_events_ map[int]MapListener
}

func (er *eventRouter) AddEventListener(evType int, listener IEventListener) bool {
	if _, ok := er.m_events_[evType]; !ok {
		er.m_events_[evType] = make(MapListener)
	}

	if _, ok := er.m_events_[evType][listener]; ok {
		return false
	}

	er.m_events_[evType][listener] = struct{}{}
	return true
}

func (er *eventRouter) DelEventListener(evType int, listener IEventListener) {
	if _, ok := er.m_events_[evType]; !ok {
		return
	}

	if _, ok := er.m_events_[evType][listener]; !ok {
		return
	}

	delete(er.m_events_[evType], listener)
}

func (er *eventRouter) DoEvent(evType int, obj interface{}, content interface{}) {
	if _, ok := er.m_events_[evType]; !ok {
		return
	}

	event := &CEvent{
		Type:    evType,
		Obj:     obj,
		Content: content,
	}

	for l, _ := range er.m_events_[evType] {
		l.OnEvent(event)
	}
}

func NewEventRouter() IEventRouter {
	ev_router := &eventRouter{
		m_events_: make(map[int]MapListener),
	}

	return ev_router
}