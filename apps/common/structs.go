package common

import (
	"bytes"
	"encoding/binary"
	"io"
)

type IMsgHead interface {
	GetMsgId() uint32
}

//外网消息头部
type ProtocolClientHead struct {
	Msg_id_ uint32
	Seq_ uint32
	Uid_ uint64
}

func(h *ProtocolClientHead) GetMsgId() uint32 {
	return h.Msg_id_
}

//内网消息头部，支持广播
type ProtocolInnerHead struct {
	Msg_id_ uint32
	Seq_ uint32
	Uid_lst_ []uint64
}

func(h *ProtocolInnerHead) GetMsgId() uint32 {
	return h.Msg_id_
}

func(head *ProtocolInnerHead) Read(buf io.Reader, order binary.ByteOrder) (err error) {
	err = binary.Read(buf, order, &head.Msg_id_)
	if err != nil {
		return
	}

	err = binary.Read(buf, order,  &head.Seq_)
	if err != nil {
		return
	}

	var user_num uint32
	err = binary.Read(buf, order, &user_num)
	if err != nil {
		return
	}

	if user_num > 0 {
		head.Uid_lst_ = make([]uint64, user_num)
		err = binary.Read(buf, order, head.Uid_lst_)
		if err != nil {
			return
		}
	}
	return
}

func(head *ProtocolInnerHead) Write(order binary.ByteOrder) ([]byte, error) {
	buf := bytes.NewBuffer(nil)
	err := binary.Write(buf, order, head.Msg_id_)
	if err != nil {
		return nil, err
	}

	err = binary.Write(buf, order, head.Seq_)
	if err != nil {
		return  nil, err
	}

	//修正一下数量，如果是0就代表广播
	user_num := uint32(len(head.Uid_lst_))
	err = binary.Write(buf, order, user_num)
	if err != nil {
		return  nil, err
	}

	if user_num > 0 {
		err = binary.Write(buf, order, head.Uid_lst_)
		if err != nil {
			return  nil, err
		}
	}

	return buf.Bytes(), nil
}

func ClientToInnerHead(head IMsgHead) IMsgHead {
	if _, ok := head.(*ProtocolInnerHead);ok {
		return head
	}

	chead := head.(*ProtocolClientHead)
	return &ProtocolInnerHead{
		Msg_id_:chead.Msg_id_,
		Seq_: chead.Seq_,
		Uid_lst_: []uint64{ chead.Uid_ },
	}
}

func InnerToClientHead(head IMsgHead) IMsgHead {
	if _, ok := head.(*ProtocolClientHead);ok {
		return head
	}

	ihead := head.(*ProtocolInnerHead)
	var uid uint64
	if len(ihead.Uid_lst_) > 0 {
		uid = ihead.Uid_lst_[0]
	}

	return &ProtocolClientHead{
		Msg_id_:ihead.Msg_id_,
		Seq_: ihead.Seq_,
		Uid_: uid,
	}
}
