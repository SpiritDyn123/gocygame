package strategy

type directSelector struct {
	BaseSelector
}

func (s *directSelector) Select() interface{} {
	obj, ok := s.m_objs_[s.idToString(s.cur_id_)]
	if !ok {
		return nil
	}

	return obj
}
