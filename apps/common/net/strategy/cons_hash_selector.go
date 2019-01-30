package strategy

import (
	"github.com/SpiritDyn123/gocygame/libs/utils"
)

type consHashSelector struct {
	BaseSelector
	hash_ring_ *utils.ConsistentHashRing
}

func (s *consHashSelector) Select() interface{} {
	str_id := s.idToString(s.cur_id_)

	id := s.hash_ring_.GetNode(str_id)
	obj, ok := s.m_objs_[id]
	if ok {
		return obj
	}
	return nil
}

func (s *consHashSelector) AddElement(id interface{}, obj interface{}) {
	str_id := s.idToString(id)
	s.m_objs_[str_id] = obj

	s.hash_ring_.AddNode(str_id, 1)
}

func (s *consHashSelector) RemoveElement(id interface{}, obj interface{}) {
	str_id := s.idToString(id)
	delete(s.m_objs_, str_id)

	s.hash_ring_.RemoveNode(str_id)
}

func (s *consHashSelector) SetElementId(id interface{}) {
	s.cur_id_ = id
}