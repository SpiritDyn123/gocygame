package common

import (
	"encoding/binary"
	"time"
)


const (
	Chanrpc_key_tcp_accept 	= "Chanrpc_key_tcp_accept"
	Chanrpc_key_tcp_recv		= "Chanrpc_key_tcp_recv"
	Chanrpc_key_tcp_close		= "Chanrpc_key_tcp_close"

	Chanrpc_key_tcp_inner_accept 	= "Chanrpc_key_tcp_inner_accept"
	Chanrpc_key_tcp_inner_recv		= "Chanrpc_key_tcp_inner_recv"
	Chanrpc_key_tcp_inner_close		= "Chanrpc_key_tcp_inner_close"
)

const(
	Server_state_begin = 1 + iota  	//启动中
	Server_state_start				//启动成功
	Server_state_end					//关闭
)

//配置一些svr的公共默认值
var (
	Default_Chan_Server_Len = 10000
	Default_Go_Server_Len = 1000

	Default_Net_Endian binary.ByteOrder = binary.BigEndian
	Default_Net_Head_Len = 4
	Default_Send_Chan_Len = 100
	Default_Svr_Send_Chan_Len = 100
	Default_Svr_Logic_time = time.Millisecond * 50

	Default_Svr_TTL = time.Second * 10
	Default_Svr_Timeout = time.Second * 30
)