package strategy

import (
	"github.com/SpiritDyn123/gocygame/libs/utils"
	"fmt"
)

type StrategyType int
const (
	Hash = StrategyType(1) + iota  //hash
	Cons_hash        //一致性hash
	Direct           //指定
)

type Selector interface {
	Select() interface{}
	AddElement(interface{}, interface{})
	RemoveElement(interface{}, interface{})
	SetElementId(interface{})
}

type BaseSelector struct {
	m_objs_ map[string]interface{}
	cur_id_ interface{}
}

func (s *BaseSelector) idToString(id interface{}) string {
	return fmt.Sprintf("%v", id)
}

func (s *BaseSelector) AddElement(id interface{}, obj interface{}) {
	s.m_objs_[s.idToString(id)] = obj
}

func (s *BaseSelector) RemoveElement(id interface{}, obj interface{}) {
	delete(s.m_objs_, s.idToString(id))
}

func (s *BaseSelector) SetElementId(id interface{}) {
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