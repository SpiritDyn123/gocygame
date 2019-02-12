# -*- coding: utf-8 -*-


Server_info = {
    "develop": {
        "name":"开发环境",
        "cond":("10.216.251.97", 7700),
        "guest":"http://preauth.shhzcj.com/api/client/guest",
        "user": "http://preauth.shhzcj.com/api/client/login"
    },
    "spirit": {
        "name":"开发(testlogin)",
        "cond":("10.216.251.97", 7700),
        "guest": "http://192.168.158.50:8002/api/client/guest",
        "user": "http://192.168.158.50:8002/api/client/login"
    },
    "test": {
        "name":"测试环境",
        "cond":("10.216.251.8", 7700),
        "guest": "http://192.168.158.50:8002/api/client/guest",
        "user": "http://192.168.158.50:8002/api/client/login"
    },
    "pre": {
        "name":"预发布环境",
        "cond":("10.216.251.24", 7700),
       "guest": "http://preauth.shhzcj.com/api/client/guest",
        "user": "http://preauth.shhzcj.com/api/client/login"
		#"guest": "http://auth.shhzcj.com/api/client/guest",
        #"user": "http://auth.shhzcj.com/api/client/login"
    },
    "product": {
        "name":"生产环境",
        "cond":("stock-gw.shhzcj.com", 7700),
        "guest": "http://auth.shhzcj.com/api/client/guest",
        "user": "http://auth.shhzcj.com/api/client/login"
    },
}

Rsa_pub_Key = "-----BEGIN PUBLIC KEY-----\n"\
" MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCuG53rp3kSO9Fnnp4Y1\n"\
"1leyTGyK+6jT68t69LMH8PYXsl1dq7RPRSPBfm8jCGgWhJs0P//ytjwmr0ZVH19\n"\
"W7WmB0aETfWCM4kraNh7h4ve7o+/mlhTibugESBVGh1cDreqTexKJZ1Ja2LP\n"\
"OQOvsvqWZHHfsi36VkaVbGHQTgJObwIDAQAB\n"\
"-----END PUBLIC KEY-----\n"

def Server_name(env):
    return Server_info[env]["name"]

def Tcp_ser_addr(env):
    return Server_info[env]["cond"]

def Http_guest_url(env):
    return Server_info[env]["guest"]

def Http_user_url(env):
    return Server_info[env]["user"]

Aes_key = '0123456789abcdef'            #aes key
Log_file = 'python_client_recv_msg.log' #日志路径
Msg_show_len_limit = 2000 #消息显示长度的限制

import sys
sys.path.append("./proto/")
from bidding_pb2 import *
from kline_pb2 import *
from publish_pb2 import *
from ret_base_pb2 import *
from search_pb2 import *
from stock_basinfo_pb2 import *
from stock_code_pb2 import *
from user_pb2 import *
from zhubi_pb2 import *
from zhibiao_pb2 import *
from short_motion_pb2 import *
from rise_fall_monitor_pb2 import *

KEEPALIVE_CMD = 0
CMD_FIRST_LOGIN = 100
CMD_GET_TIME = 101

CMD_MSG_USER_START = 1000
CMD_MSG_USER_LOGIN = 1001
CMD_MSG_UPLOAD_LOGIN_INFO = 1002
CMD_MSG_KICK_USER = 1003
CMD_MSG_USER_END = 4999

CMD_MSG_SYS_START = 5000
CMD_MSG_PERMISSION_ERR 	= 5000;
CMD_MSG_SYS_END = 9999

CMD_MSG_QUOTE_START = 10000
CMD_GET_STOCKCODE   = 10001
CMD_GET_EXPONENT    = 10002
CMD_BATCHGET_STOCK  = 10003
CMD_STOCK_OPTIONAL  = 10004
CMD_GET_KLINE       = 10005
CMD_GET_ZHUBI       = 10006
CMD_MSG_MONEYOPTIONAL  = 10007
CMD_MSG_BATCHGETMONEY = 10008
CMD_MSG_TIMEKLINE 	    = 10009
CMD_MSG_STOCK_RISE_FALL_INFO 	=	10010
CMD_MSG_BIDDING_OPTIONAL =   10011
CMD_MSG_BATCH_GET_BIDDING =	10012
CMD_MSG_BIDDING_BLOCK_OPTIONAL =   10013
CMD_MSG_BATCH_GET_BIDDING_BLOCK =	10014
CMD_MSG_GET_BIDDING_QUOTE_KLINE =	10015
CMD_MSG_GET_BIDDING_BLOCK_KLINE =	10016
CMD_MSG_GET_BLOCK_STOCKS =	10017
CMD_MSG_BATCH_MONEY_ABNORMAL =	10018
CMD_MSG_BLOCK_MONEY_ABNORMAL =	10019
CMD_MSG_MONEY_ABNORMAL 		 =	10020
CMD_MSG_BATCH_SHORT_MOTION   = 10021
CMD_MSG_RFMONITOR_NUM_INFO  = 10022
CMD_MSG_BATCH_RFMONITOR = 10023
CMD_MSG_RFM_GROUP_TYPES_INFO = 10024
CMD_MSG_GET_FENJIA			 = 10025
CMD_MSG_MONEY_FIELD_LINE		= 10026
CMD_MSG_QUOTE_END = 14999

CMD_MSG_SEARCH_START = 15000
CMD_MSG_QUERY_CODE = 15001
CMD_MSG_SEARCH_END = 15999

CMD_MSG_MONGO_START  = 16000
CMD_MSG_MONGO_CODE   = 16001
CMD_MSG_MONGO_END  = 16999

CMD_MSG_PUBLISH_START  = 17000
CMD_MSG_PUBLISH		  = 17001
CMD_MSG_PUBLISH_END  = 17999

CMD_MSG_ZHIBIAO_START = 18000
CMD_MSG_ZHIBIAO = 18001
CMD_MSG_ZHIBIAO_HJJJ = 18001				# 指标 黄金狙击：见底出击，筹码分布，趋势拐点	GetZhibiaoRequest
CMD_MSG_ZHIBIAO_DCXG = 18002				# 指标 多彩选股：黄蓝区间，庄家控盘，操盘提醒	GetZhibiaoRequest
CMD_MSG_ZHIBIAO_END = 18049

Proto_msg_map = {
    #KEEPALIVE_CMD: [, ],
    CMD_FIRST_LOGIN: [FirstConnectRequest, FirstConnectResponse],
    CMD_MSG_USER_LOGIN: [UserLoginRequest, UserLoginResponse],
	CMD_GET_TIME: [GetTimeRequest, GetTimeResponse],
	
    CMD_GET_STOCKCODE: [GetStockCodeRequest, GetStockCodeResponse],
    CMD_BATCHGET_STOCK: [BatchGetStockRequest, BatchGetStockResponse],
    CMD_STOCK_OPTIONAL: [StockOptionalRequest, StockOptionalResponse],
    CMD_GET_KLINE: [GetKLineRequest, GetKLineResponse],
    CMD_GET_ZHUBI: [GetZhubiRequest, GetZhubiResponse],
    CMD_MSG_MONEYOPTIONAL: [MoneyOptionalRequest, MoneyOptionalResponse],

    CMD_MSG_BATCHGETMONEY: [BatchGetMoneyRequest, BatchGetMoneyResponse],
    CMD_MSG_TIMEKLINE: [GetTimeKLineRequest, GetTimeKLineResponse],
    CMD_MSG_STOCK_RISE_FALL_INFO: [GetStockRiseFallInfoRequest, GetStockRiseFallInfoResponse],
    CMD_MSG_BIDDING_OPTIONAL: [BiddingOptionalRequest, BiddingOptionalResponse],
    CMD_MSG_BATCH_GET_BIDDING: [BatchGetBiddingRequest, BatchGetBiddingResponse],
    CMD_MSG_BIDDING_BLOCK_OPTIONAL: [BiddingBlockOptionalRequest, BiddingBlockOptionalResponse],

    CMD_MSG_BATCH_GET_BIDDING_BLOCK: [BatchGetBiddingBlockRequest, BatchGetBiddingBlockResponse],
    CMD_MSG_GET_BIDDING_QUOTE_KLINE: [GetBiddingQuoteKlineRequest, GetBiddingQuoteKlineResponse],
    CMD_MSG_GET_BIDDING_BLOCK_KLINE: [GetBiddingBlockKlineRequest, GetBiddingBlockKlineResponse],
    CMD_MSG_GET_BLOCK_STOCKS: [GetStocksOfBlockRequest, GetStocksOfBlockResponse],
    CMD_MSG_BATCH_MONEY_ABNORMAL: [BatchGetMoneyAbnormalRequest, BatchGetMoneyAbnormalResponse],
    CMD_MSG_BLOCK_MONEY_ABNORMAL: [BlockMoneyAbnoramlRequest,BlockMoneyAbnoramlResponse],
    CMD_MSG_MONEY_ABNORMAL: [MoneyAbnormalOptionalRequest, MoneyAbnormalOptionalResponse],

    CMD_MSG_RFMONITOR_NUM_INFO: [RFMonitorNumInfoOptionalRequest, RFMonitorNumInfoOptionalResponse],
    CMD_MSG_BATCH_RFMONITOR: [BatchRFMonitorRequest, BatchRFMonitorResponse],
    CMD_MSG_RFM_GROUP_TYPES_INFO: [RFMGroupTypesRequest, RFMGroupTypesResponse],
    CMD_MSG_GET_FENJIA: [GetFenJiaRequest, GetFenJiaResponse],			 			#GetFenJiaRequest
    CMD_MSG_MONEY_FIELD_LINE: [MoneyFieldLineRequest, MoneyFieldLineResponse],					#MoneyFieldLineRequest

    CMD_MSG_QUERY_CODE: [QueryCodeRequest, QueryCodeResponse],
    CMD_MSG_PUBLISH: [PublishRequest, PublishResponse],

    CMD_MSG_ZHIBIAO: [GetZhibiaoRequest, GetZhibiaoResponse],
    CMD_MSG_ZHIBIAO_HJJJ: [GetZhibiaoRequest, GetZhibiaoResponse],
    CMD_MSG_ZHIBIAO_DCXG: [GetZhibiaoRequest, GetZhibiaoResponse],
    CMD_MSG_BATCH_SHORT_MOTION:[BatchShortMotionRequest,BatchShortMotionResponse],
}
