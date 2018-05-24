package tcp

import (
	"math"
	"net"
	"io"
	"encoding/binary"
	"errors"
)

type CloseSendChan interface {
	CloseEnd(<-chan interface{})
}

type Codec interface {
	Marshal(v interface{}) ([]byte, error)
	Unmarshal(data []byte) (interface{}, error)
}

type Protocol interface {
	NewCodec() Codec
}

type ProtocolFunc func() Codec
func (pf ProtocolFunc) NewCodec() Codec {
	return pf()
}


type MsgParser struct {
	lenMsgLen    int
	maxRecvMsgLen    uint32
	maxSendMsgLen    uint32
	littleEndian bool
	includeHead bool
}

func NewMsgParser() *MsgParser {
	p := new(MsgParser)
	p.lenMsgLen = 2
	p.maxRecvMsgLen = 4096
	p.maxSendMsgLen = 4096
	p.littleEndian = false

	return p
}

// It's dangerous to call the method on reading or writing
func (p *MsgParser) SetMsgLen(lenMsgLen int, maxRecvMsgLen uint32, maxSendMsgLen uint32) {
	if lenMsgLen == 1 || lenMsgLen == 2 || lenMsgLen == 4 {
		p.lenMsgLen = lenMsgLen
	}
	if maxRecvMsgLen != 0 {
		p.maxRecvMsgLen = maxRecvMsgLen
	}
	if maxSendMsgLen != 0 {
		p.maxSendMsgLen = maxSendMsgLen
	}

	var max uint32
	switch p.lenMsgLen {
	case 1:
		max = math.MaxUint8
	case 2:
		max = math.MaxUint16
	case 4:
		max = math.MaxUint32
	}
	if p.maxRecvMsgLen > max {
		p.maxRecvMsgLen = max
	}
	if p.maxSendMsgLen > max {
		p.maxSendMsgLen = max
	}
}

// It's dangerous to call the method on reading or writing
func (p *MsgParser) SetByteOrder(littleEndian bool) {
	p.littleEndian = littleEndian
}

func (p *MsgParser) SetIncludeHead(ih bool) {
	p.includeHead = ih
}
// goroutine safe
func (p *MsgParser) Read(conn net.Conn) ([]byte, error) {
	var b [4]byte
	bufMsgLen := b[:p.lenMsgLen]
	// read len
	if _, err := io.ReadFull(conn, bufMsgLen); err != nil {
		return nil, err
	}
	// parse len
	var msgLen uint32
	switch p.lenMsgLen {
	case 1:
		msgLen = uint32(bufMsgLen[0])
	case 2:
		if p.littleEndian {
			msgLen = uint32(binary.LittleEndian.Uint16(bufMsgLen))
		} else {
			msgLen = uint32(binary.BigEndian.Uint16(bufMsgLen))
		}
	case 4:
		if p.littleEndian {
			msgLen = binary.LittleEndian.Uint32(bufMsgLen)
		} else {
			msgLen = binary.BigEndian.Uint32(bufMsgLen)
		}
	}

	//是否包含长度
	if p.includeHead {
		msgLen -= uint32(p.lenMsgLen)
	}

	// check len
	if msgLen > p.maxRecvMsgLen {
		return nil, errors.New("read message too long")
	}

	if msgLen > 0 {
		// data
		msgData := make([]byte, msgLen)
		if _, err := io.ReadFull(conn, msgData); err != nil {
			return nil, err
		}
		return msgData, nil
	} else {
		return []byte{}, nil
	}
}

// goroutine safe
func (p *MsgParser) Write(conn net.Conn, args ...[]byte) error {
	// get len
	var msgLen uint32
	for i := 0; i < len(args); i++ {
		msgLen += uint32(len(args[i]))
	}

	// check len
	if msgLen > p.maxSendMsgLen {
		return errors.New("write message too long")
	}

	msg := make([]byte, uint32(p.lenMsgLen)+msgLen)
	//是否包含长度
	if p.includeHead {
		msgLen += uint32(p.lenMsgLen)
	}

	// write len
	switch p.lenMsgLen {
	case 1:
		msg[0] = byte(msgLen)
	case 2:
		if p.littleEndian {
			binary.LittleEndian.PutUint16(msg, uint16(msgLen))
		} else {
			binary.BigEndian.PutUint16(msg, uint16(msgLen))
		}
	case 4:
		if p.littleEndian {
			binary.LittleEndian.PutUint32(msg, msgLen)
		} else {
			binary.BigEndian.PutUint32(msg, msgLen)
		}
	}

	// write data
	l := p.lenMsgLen
	for i := 0; i < len(args); i++ {
		copy(msg[l:], args[i])
		l += len(args[i])
	}

	conn.Write(msg)

	return nil
}
