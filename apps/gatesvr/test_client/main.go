package main

import (
	"encoding/binary"
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/apps/common/net/codec"
	"github.com/SpiritDyn123/gocygame/libs/chanrpc"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"time"
)

func main() {

	chan_s := chanrpc.NewServer(111)
	chan_s.Register(common.Chanrpc_key_tcp_inner_accept, func(args []interface{}){
		fmt.Println("connect", args)
		head := &common.ProtocolClientHead{
			Uid_: 100,
			Seq_: 100,
			Msg_id_: 1001,
		}
		args[0].(*tcp.Session).Send(head, []byte{1, 2, 55})
	})

	chan_s.Register(common.Chanrpc_key_tcp_inner_close, func(args []interface{}){
		fmt.Println("closed", args)
	})

	chan_s.Register(common.Chanrpc_key_tcp_inner_recv, func(args []interface{}){
		fmt.Println("recv", args)
	})

	msg_parser := tcp.NewMsgParser()
	msg_parser.SetByteOrder(false)
	msg_parser.SetIncludeHead(true)
	msg_parser.SetMsgLen(4, 111111, 11111)
	cli := &tcp.Client{
		Network:"tcp",
		Addr: "127.0.0.1:11000",
		DialTimeOut: time.Second * 3,
		ConnectNum:1,
		Protocol: &codec.ProtoClientProtocol{Endian_: binary.BigEndian},
		MsgParser: msg_parser,
		SendChanSize: 100,
		ChanServer: chan_s,
		ConnectKey: common.Chanrpc_key_tcp_inner_accept,
		ClosedKey: common.Chanrpc_key_tcp_inner_close,
		RecvKey: common.Chanrpc_key_tcp_inner_recv,
		UseTLS: false,
	}

	cli.Connect()
	for {
		select {
		case ci := <- chan_s.ChanCall:
			chan_s.Exec(ci)
		}
	}
}

