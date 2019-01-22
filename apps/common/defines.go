package common

import (
	"encoding/binary"
	"time"
)

const (
	Svr_type_gate = 1 + iota  //网关服务器
	Svr_type_gs				//逻辑服务器
	Svr_type_world  			//大厅服务器
	Svr_type_login  			//登陆服务器
	Svr_type_chat   			//聊天服务器
	Svr_type_db     			//DBproxy服务器
	Svr_type_manager          //管理服务器
)

const (
	Chanrpc_key_tcp_accept 	= "Chanrpc_key_tcp_accept"
	Chanrpc_key_tcp_recv		= "Chanrpc_key_tcp_recv"
	Chanrpc_key_tcp_close		= "Chanrpc_key_tcp_close"

	Chanrpc_key_tcp_inner_accept 	= "Chanrpc_key_tcp_inner_accept"
	Chanrpc_key_tcp_inner_recv		= "Chanrpc_key_tcp_inner_recv"
	Chanrpc_key_tcp_inner_close		= "Chanrpc_key_tcp_inner_close"
)

//配置一些svr的公共默认值
var (
	Default_Chan_Server_Len = 10000
	Default_Go_Server_Len = 1000

	Default_Net_Endian = binary.BigEndian
	Default_Net_Head_Len = 4
	Default_Send_Chan_Len = 100
	Default_Svr_Logic_time = time.Millisecond * 50
)