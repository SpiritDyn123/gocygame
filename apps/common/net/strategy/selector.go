package strategy

import "github.com/SpiritDyn123/gocygame/libs/utils"

type StrategyType int
const (
	Hash = StrategyType(1) + iota  //hash
	Cons_hash        //一致性hash
	Direct           //指定
)

type Selector interface {
	Select() interface{}
	AddElement(string, interface{})
	RemoveElement(string, interface{})
	SetElementId(string)
}

type BaseSelector struct {
	m_objs_ map[string]interface{}
	cur_id_ string
}

func (s *BaseSelector) AddElement(id string, obj interface{}) {
	s.m_objs_[id] = obj
}

func (s *BaseSelector) RemoveElement(id string, obj interface{}) {
	delete(s.m_objs_, id)
}

func (s *BaseSelector) SetElementId(id string) {
	s.cur_id_ = id
}

func CreateSelector(stype StrategyType, cfg interface{}) Selector{
	switch stype {
	case Cons_hash:
		s := &consHashSelector{
			BaseSelector:BaseSelector{
				m_objs_: make(map[string]interface{}),
		}}
		s.hash_ring_ = utils.NewConsistentHashRing(cfg.(int))
		return s
	case Hash:
		return &hashSelector{BaseSelector:BaseSelector{
			m_objs_: make(map[string]interface{}),
		}}
	case Direct:
		return &directSelector{BaseSelector:BaseSelector{
			m_objs_: make(map[string]interface{}),
		}}
	}

	return nil
}