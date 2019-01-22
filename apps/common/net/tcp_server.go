package net

import (
	"encoding/binary"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/libs/chanrpc"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"time"
)

//websocket 和普通tcp 区别在于不用自己拼凑长度和设置大小端，所以msg_parser 不需要(nil)
func CreateTcpServer(svr_cfg *common.Cfg_Json_SvrBase, tls *common.Cfg_Json_TLS, protocol tcp.Protocol, len_msg int, endian binary.ByteOrder,
	send_chan_size int, chan_ser *chanrpc.Server) (net_ser tcp.INetServer, err error) {

	msg_parser := tcp.NewMsgParser()
	msg_parser.SetMsgLen(len_msg, uint32(svr_cfg.Svr_in_bytes_), uint32(svr_cfg.Svr_out_bytes_))
	msg_parser.SetIncludeHead(true)
	msg_parser.SetByteOrder(endian == binary.LittleEndian)
	if tls != nil {
		if tls.Ws_ {//websocket
			net_ser, err = tcp.CreateWSServer("tcp", svr_cfg.Svr_addr_, tls.Cert_file_, tls.Key_file_, protocol, svr_cfg.Svr_in_bytes_,
				svr_cfg.Svr_out_bytes_, time.Second * 3, time.Second *3,
				send_chan_size, chan_ser, common.Chanrpc_key_tcp_accept, common.Chanrpc_key_tcp_recv, common.Chanrpc_key_tcp_close)
		} else { //tls
			net_ser, err = tcp.CreateTLSServer("tcp", svr_cfg.Svr_addr_, tls.Cert_file_, tls.Key_file_,
				protocol, msg_parser, send_chan_size, chan_ser, common.Chanrpc_key_tcp_accept, common.Chanrpc_key_tcp_recv, common.Chanrpc_key_tcp_close)
		}
	} else {
		net_ser, err = tcp.CreateTcpServer("tcp", svr_cfg.Svr_addr_, protocol, msg_parser, send_chan_size, chan_ser,
			common.Chanrpc_key_tcp_accept, common.Chanrpc_key_tcp_recv, common.Chanrpc_key_tcp_close)
	}

	return
}
