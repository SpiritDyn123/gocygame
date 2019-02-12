# -*- coding: utf-8 -*-

from stock_basinfo_pb2 import  *
from kline_pb2 import *
from public_message_pb2 import *
#
# message GetKLineRequest{
# 	required	string	stock_name = 1;		//股票代码 sh600000
# 	required	int32		kline_type = 2;		//K线类型
# 	required	int32 		start_time = 3;		//指定起始点位置
# 	required	int32 		count = 4;				//表明请求拉取的根数
# 	required	bool		is_desc = 5;			//true 表示降序(倒拉)
# }
# message GetKLineResponse{
# 	required	Ret ret = 1;
# 	repeated	KLine	kline_info = 2;			//K线基础信息
# 	optional    string  stock_code = 3;			//K线代码
# 	optional	int32		kline_type = 4;		//K线类型
# }


msg = StockBase()

ret = msg.DESCRIPTOR.__dict__["fields_by_name"]['rate']
stk = msg.DESCRIPTOR.__dict__["fields_by_name"]['trading_day']

import json

str_type = type(getattr(msg, "trading_day")).__name__
print(str_type)

import json
data = '{"abc":1}'
obj = json.loads(data)


class obj1(object):
    def __init__(self):
        self.id = 0
        self.dict_data = 0

class obj2(object):
    def __init__(self):
        #self.obj1 = obj1()
        self.st = ""


data = "sdsd_[dsfd]"
data = data[:data.index("[")]
print(data)

import time
print(time.time())