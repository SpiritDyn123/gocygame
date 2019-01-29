package strategy

import (
	"github.com/SpiritDyn123/gocygame/libs/utils"
)

type consHashSelector struct {
	BaseSelector
	hash_ring_ *utils.ConsistentHashRing
}

func (s *consHashSelector) Select() interface{} {
	id := s.hash_ring_.GetNode(s.cur_id_)
	obj, ok := s.m_objs_[id]
	if !ok {
		return obj
	}

	return nil
}

func (s *consHashSelector) AddElement(id string, obj interface{}) {
	s.m_objs_[id] = obj
	s.hash_ring_.AddNode(id, 1)
}

func (s *consHashSelector) RemoveElement(id string, obj interface{}) {
	delete(s.m_objs_, id)
	s.hash_ring_.RemoveNode(id)
}

func (s *consHashSelector) SetElementId(id string) {
	s.cur_id_ = id
}