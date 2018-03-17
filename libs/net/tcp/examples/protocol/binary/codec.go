package binary

import "fmt"

type BinaryCodec struct {

}

func (bc *BinaryCodec) Marshal(v interface{}) ([]byte, error) {
	data, ok := v.([]byte)
	if !ok {
		return nil, fmt.Errorf("Marshal msg type must be []bye")
	}
	return data, nil
}

func (bc *BinaryCodec) Unmarshal(data []byte) (interface{}, error) {
	return data, nil
}