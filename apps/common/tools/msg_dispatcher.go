package tools

import (
	"github.com/golang/protobuf/proto"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"errors"
)

//sink 可以是玩家或者session， head是消息的头部 msg是proto包体
type MsgCallBack func(sink interface{}, head common.IMsgHead, msg proto.Message)
type IMsgDispatcher interface {
	Register(mid ProtoMsg.EmMsgId, msg proto.Message, cb MsgCallBack) error
	Dispatch(sink interface{}, head common.IMsgHead, msg_data []byte) error
}

type cbInfo struct {
	cb MsgCallBack
	msg proto.Message
}

type msgDispatcher struct {
	m_reg_cbs_ map[uint32]*cbInfo
}

func(disp *msgDispatcher) Register(mid ProtoMsg.EmMsgId, msg proto.Message, cb MsgCallBack) (err error) {
	disp.m_reg_cbs_[uint32(mid)] = &cbInfo{
		cb: cb,
		msg: msg,
	}

	return
}

var Error_not_found = errors.New("handle not found")

func(disp *msgDispatcher) Dispatch(sink interface{}, head common.IMsgHead, msg_data []byte) (err error){
	mid := head.GetMsgId()

	cb_info, ok := disp.m_reg_cbs_[mid]
	if !ok {
		return Error_not_found
	}

	if msg_data != nil {
		err = proto.Unmarshal(msg_data, cb_info.msg)
		if err != nil {
			return
		}
		cb_info.cb(sink, head, cb_info.msg)
	} else {
		cb_info.cb(sink, head, nil)
	}

	return
}

func CreateMsgDispatcher() IMsgDispatcher {
	return &msgDispatcher{
		m_reg_cbs_: make(map[uint32]*cbInfo),
	}
}
