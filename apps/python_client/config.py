# -*- coding: utf-8 -*-


Server_info = {
    "develop": {
        "name":"开发环境",
        "gate":("127.0.0.1", 11000),
    },
}

def Server_name(env):
    return Server_info[env]["name"]

def Tcp_ser_addr(env):
    return Server_info[env]["gate"]

def Http_guest_url(env):
    return Server_info[env]["guest"]

def Http_user_url(env):
    return Server_info[env]["user"]

Log_file = 'python_client_recv_msg.log' #日志路径
Msg_show_len_limit = 2000 #消息显示长度的限制

import sys
# sys.path.append("./proto/")
# from cs_protoid_pb2 import  *

from proto.cs_protoid_pb2 import *
from proto.login_pb2 import *
Proto_msg_map = {
    CS_MSG_PLAYER_LOGIN: [PbCsPlayerLoginReqMsg, PbCsPlayerLoginResMsg], #登陆协议
}
