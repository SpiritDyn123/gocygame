package binary

import (
	"io"
	"github.com/funny/link"
	"fmt"
	"io/ioutil"
)

type BinaryProtocl struct {

}

func (bp *BinaryProtocl) NewCodec(rw io.ReadWriter) (link.Codec, error) {
	return &binaryCodec{
		rw :rw,
	}, nil
}

type binaryCodec struct {
	rw io.ReadWriter
}

func (bc *binaryCodec) Receive() (msg interface{}, err error) {
	data, err := ioutil.ReadAll(bc.rw)
	if err == nil {
		return string(data), nil
	}

	return data, err
}

func (bc *binaryCodec) Send(msg interface{}) error {
	msgstr, ok := msg.(string)
	if !ok {
		return fmt.Errorf("msg:%v is not string", msg)
	}

	_, err := bc.rw.Write([]byte(msgstr))
	return err
}

func (bc *binaryCodec) Close() error {
	if closer, ok := bc.rw.(io.Closer);ok {
		return closer.Close()
	}

	return nil
}