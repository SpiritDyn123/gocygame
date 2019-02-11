package tools

import (
	"github.com/golang/protobuf/proto"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"errors"
	"reflect"
)

//sink 可以是玩家或者session， head是消息的头部 msg是proto包体
type MsgCallBack func(sink interface{}, head common.IMsgHead, msg interface{})
type MsgTransmitCallBack  func(sink interface{}, head common.IMsgHead, msg interface{})
type IMsgDispatcher interface {
	Register(mid uint32, msg proto.Message, cb MsgCallBack) error
	Dispatch(sink interface{}, head common.IMsgHead, msg_data []byte) error
}

type cbInfo struct {
	cb MsgCallBack
	msg reflect.Type
}

type msgDispatcher struct {
	transmit_cb_ MsgTransmitCallBack
	m_reg_cbs_ map[uint32]*cbInfo
}

func(disp *msgDispatcher) Register(mid uint32, msg proto.Message, cb MsgCallBack) (err error) {
	disp.m_reg_cbs_[uint32(mid)] = &cbInfo{
		cb: cb,
		msg: reflect.TypeOf(msg),
	}

	return
}

var Error_not_found = errors.New("handle not found")

func(disp *msgDispatcher) Dispatch(sink interface{}, head common.IMsgHead, msg_data []byte) (err error){
	mid := head.GetMsgId()

	cb_info, ok := disp.m_reg_cbs_[mid]
	if !ok {
		if disp.transmit_cb_ != nil {
			disp.transmit_cb_(sink, head, msg_data)
			return
		}
		return Error_not_found
	}

	if msg_data != nil && cb_info.msg != nil {
		//new 一条消息
		msg := reflect.New(cb_info.msg.Elem()).Interface().(proto.Message)
		err = proto.Unmarshal(msg_data, msg)
		if err != nil {
			return
		}
		cb_info.cb(sink, head, msg)
	} else {
		cb_info.cb(sink, head, msg_data)
	}

	return
}

func CreateMsgDispatcher() IMsgDispatcher {
	return &msgDispatcher{
		m_reg_cbs_: make(map[uint32]*cbInfo),
	}
}

func CreateMsgDispatcherWithTransmit(cb MsgTransmitCallBack) IMsgDispatcher {
	return &msgDispatcher{
		m_reg_cbs_: make(map[uint32]*cbInfo),
		transmit_cb_: cb,
	}
}