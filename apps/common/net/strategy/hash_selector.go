package strategy

import (
	"hash/crc32"
)

type hashSelector struct {
	BaseSelector
}

func (s *hashSelector) Select() interface{} {
	indx := int(crc32.ChecksumIEEE([]byte(s.idToString(s.cur_id_))) % uint32(len(s.m_objs_)))
	for _, obj := range s.m_objs_ {
		if indx == 0 {
			return obj
		}

		indx--
	}

	return nil
}

