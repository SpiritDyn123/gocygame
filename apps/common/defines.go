package common


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
)