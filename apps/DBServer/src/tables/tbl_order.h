/*===============================================================
* @Author: car
* @Created Time : 2018年06月12日 星期二 10时40分51秒
*
* @File Name: tbl_order.h
* @Description:
*
================================================================*/
#pragma once

#include "../core/dbtable.h"

class TblOrder
	: public DBTable<1, 1> 
{
public:
	using DBTable::DBTable;
	//TblOrder(SQLConnection& conn, const char* db, const char* tbl);
	int Add(const std::string& cp_trade_no, double total_fee,
			const std::string& currency_type, const std::string& product_id,
			const std::string& create_time, const std::string& trade_time,
			const std::string& is_test_order, const std::string& channel_trade_no,
			const std::string& channel, const std::string& gems);


};

